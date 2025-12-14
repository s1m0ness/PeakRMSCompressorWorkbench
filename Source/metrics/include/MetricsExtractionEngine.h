#pragma once
#include <JuceHeader.h>
#include "AudioFileLoader.h"
#include "DataExport.h"

class Compressor;
class Metrics;

class MetricsExtractionEngine
{
public:
    struct Config
    {
        int chunkSize = 1024;
        int maxDurationMinutes = 20;  // safety cap
    };

    MetricsExtractionEngine(AudioFileLoader& loader,
        DataExport& exporter,
        Compressor& peakCompressor,
        Compressor& rmsCompressor,
        Metrics& metrics,
        juce::AudioProcessorValueTreeState& apvts,
        Config cfg);

    void run(const juce::File& selectedFile);

    bool isProcessing() const noexcept { return processing.load(); }
    double getProgress() const noexcept { return progress.load(); }

private:
    void compressAudioFile();
    void processBufferInChunks(juce::AudioBuffer<float>& grBuffer,
        juce::AudioBuffer<float>& audioBuffer,
        bool isRMS,
        Compressor& compressor);

    void triggerMetricsExtraction();
    juce::String buildMetricsReport() const;

    float getParam(const juce::String& id) const;
    juce::String formatParameterBlock(const juce::String& title,
        const juce::String& prefix) const;

private:
    // Dependencies
    AudioFileLoader& loader;
    DataExport& exporter;
    Compressor& peakCompressor;
    Compressor& rmsCompressor;
    Metrics& metrics;
    juce::AudioProcessorValueTreeState& apvts;
    Config cfg;

    // State
    juce::File selectedFile;
    bool fileExists = false;
    double fileSampleRate = 0.0;

    // Buffers
    juce::AudioBuffer<float> uncompressedSignal;
    juce::AudioBuffer<float> peakCompressedSignal;
    juce::AudioBuffer<float> peakGainReductionSignal;
    juce::AudioBuffer<float> rmsCompressedSignal;
    juce::AudioBuffer<float> rmsGainReductionSignal;

    // UI/progress
    std::atomic<bool> processing{ false };
    std::atomic<double> progress{ 0.0 };
};
