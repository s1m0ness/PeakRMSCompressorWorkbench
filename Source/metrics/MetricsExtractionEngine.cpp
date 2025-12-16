#include "include/MetricsExtractionEngine.h"

// Include your actual headers here
#include "../dsp/include/Compressor.h"
#include "include/Metrics.h"

MetricsExtractionEngine::MetricsExtractionEngine(AudioFileLoader& l,
    DataExport& e,
    Compressor& peak,
    Compressor& rms,
    Metrics& m,
    juce::AudioProcessorValueTreeState& state,
    Config c)
    : loader(l),
    exporter(e),
    peakCompressor(peak),
    rmsCompressor(rms),
    metrics(m),
    apvts(state),
    cfg(std::move(c))
{
}

void MetricsExtractionEngine::run(const juce::File& file)
{
    processing = true;
    progress = 0.0;

    selectedFile = file;

    try
    {
        // Check if selected file exists
        if (selectedFile == juce::File{} || !selectedFile.existsAsFile())
        {
            processing = false;
            return;
        }

        juce::String err; // error message in case the extraction fails at some point

        // Load full audio file
        auto loaded = loader.loadAudioFile(selectedFile, &err);
        if (!loaded.has_value())
            throw std::runtime_error(err.toStdString());

        uncompressedSignal = std::move(loaded->buffer);
        fileSampleRate = loaded->sampleRate;

        progress = 0.3;

        // Offline compression stage (chunked)
        compressAudioFile();

        progress = 0.6;

        // Metrics computation stage
        getMetrics();

        progress = 0.8;

        // Build report text
        const auto report = buildMetricsReport();

        // Export stage (DataExport owns folder/naming/writing)
        const juce::AudioBuffer<float>* peakPtr = &peakCompressedSignal;
        const juce::AudioBuffer<float>* rmsPtr = &rmsCompressedSignal;

        if (!exporter.exportAll(selectedFile, report, peakPtr, rmsPtr, fileSampleRate, &err))
            throw std::runtime_error(err.toStdString());

        progress = 1.0;
    }
    catch (const std::exception& e)
    {
        DBG("Metrics extraction failed: " + juce::String(e.what()));
    }
    catch (...)
    {
        DBG("Unknown error during metrics extraction.");
    }

    processing = false;
}


void MetricsExtractionEngine::compressAudioFile()
{
    jassert(uncompressedSignal.getNumSamples() > 0);

    const int numSamples = uncompressedSignal.getNumSamples();
    const int numChannels = uncompressedSignal.getNumChannels();

    // Prepare clear buffers for storing compressed signals
    peakCompressedSignal.setSize(numChannels, numSamples, false, true, true);
    peakGainReductionSignal.setSize(numChannels, numSamples, false, true, true);
    rmsCompressedSignal.setSize(numChannels, numSamples, false, true, true);
    rmsGainReductionSignal.setSize(numChannels, numSamples, false, true, true);

    // fill the buffers with uncompressed signal
    peakCompressedSignal.makeCopyOf(uncompressedSignal);
    rmsCompressedSignal.makeCopyOf(uncompressedSignal);

    processBufferInChunks(peakGainReductionSignal, peakCompressedSignal, false, peakCompressor);
    processBufferInChunks(rmsGainReductionSignal, rmsCompressedSignal, true, rmsCompressor);
}

void MetricsExtractionEngine::processBufferInChunks(juce::AudioBuffer<float>& grBuffer,
    juce::AudioBuffer<float>& audioBuffer,
    bool isRMS,
    Compressor& compressor)
{
    const int numSamples = audioBuffer.getNumSamples();
    const int numChannels = audioBuffer.getNumChannels();
    const int chunkSize =  cfg.chunkSize;

    // the compressor now operates on a loaded audio signal during offline analysis
    // so the compressor settings have to reflect loaded audio parameters
    compressor.prepareForMetricsExtraction({ fileSampleRate, static_cast<uint32_t>(chunkSize), 2 });

    juce::AudioBuffer<float> chunkBuffer;
    chunkBuffer.setSize(numChannels, chunkSize, false, true, true);

    for (int start = 0; start < numSamples; start += chunkSize)
    {
        const int n = std::min(chunkSize, numSamples - start);
        if (n <= 0) continue;

        if (n < chunkSize) chunkBuffer.setSize(numChannels, n, false, true, true);

        for (int ch = 0; ch < numChannels; ++ch)
            chunkBuffer.copyFrom(ch, 0, audioBuffer, ch, start, n);

        // compressor.process() isn't called directly because the compressor is bypassed
        // during metrics extraction
        if (isRMS) compressor.applyRMSCompression(chunkBuffer, n, numChannels, true);
        else       compressor.applyPeakCompression(chunkBuffer, n, numChannels, true);


        auto& gr = compressor.getGainReductionSignal();
        for (int ch = 0; ch < numChannels; ++ch)
            grBuffer.copyFrom(ch, start, gr, ch, 0, n);

        for (int ch = 0; ch < numChannels; ++ch)
            audioBuffer.copyFrom(ch, start, chunkBuffer, ch, 0, n);
    }

    // Back to real time processing compressor settings after the compression is finished
    compressor.prepareForRealTimeProcessing();
}

void MetricsExtractionEngine::getMetrics()
{
    metrics.prepare(fileSampleRate);

    metrics.setUncompressedSignal(&uncompressedSignal);
    metrics.setPeakGainReductionSignal(&peakGainReductionSignal);
    metrics.setPeakCompressedSignal(&peakCompressedSignal);
    metrics.setRMSGainReductionSignal(&rmsGainReductionSignal);
    metrics.setRMSCompressedSignal(&rmsCompressedSignal);

    metrics.extractMetrics();
}

juce::String MetricsExtractionEngine::buildMetricsReport() const
{
    juce::String text;
    text << "Metrics Summary for: " << selectedFile.getFileName() << "\n\n";

    const auto& uncompressed = metrics.getUncompressedMetrics();
    const auto& peak = metrics.getPeakMetrics();
    const auto& rms = metrics.getRMSMetrics();

    text << uncompressed.formatMetrics();
    text << formatParameterBlock("Compression parameter values for peak detection", "peak_");
    text << peak.formatMetrics();
    text << formatParameterBlock("Compression parameter values for rms detection", "rms_");
    text << rms.formatMetrics();

    return text;
}

juce::String MetricsExtractionEngine::formatParameterBlock(const juce::String& title,
    const juce::String& prefix) const
{
    juce::String c;
    c << title << ":\n";
    c << "Threshold: " << getParam(prefix + "threshold") << ", ";
    c << "Ratio: " << getParam(prefix + "ratio") << ", ";
    c << "Knee: " << getParam(prefix + "knee") << ", ";
    c << "Attack: " << getParam(prefix + "attack") << ", ";
    c << "Release: " << getParam(prefix + "release") << ", ";
    c << "Makeup Gain: " << getParam(prefix + "makeup") << ".\n";
    return c;
}

float MetricsExtractionEngine::getParam(const juce::String& id) const
{
    if (auto* v = apvts.getRawParameterValue(id))
        return v->load();
    return 0.0f;
}
