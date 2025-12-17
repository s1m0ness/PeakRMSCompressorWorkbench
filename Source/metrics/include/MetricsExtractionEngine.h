/*
 * This file defines the MetricsExtractionEngine class, responsible for
 * orchestrating the extraction of objective evaluation metrics for audio
 * compression analysis.
 *
 * Key Features:
 * - Loads and processes audio files for evaluation.
 * - Applies both peak-based and RMS-based compression to the input signal.
 * - Collects gain reduction and compressed audio signals.
 * - Triggers metric computation and exports the resulting data.
 * - Provides progress reporting for long-running analysis tasks.
 *
 * License:
 * This file is part of the PeakRMSCompressorWorkbench project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <JuceHeader.h>
#include "AudioFileLoader.h"
#include "DataExport.h"

 //==============================================================================

class Compressor;
class Metrics;

//==============================================================================

class MetricsExtractionEngine
{
public:
    struct Config
    {
        int chunkSize = 1024; // default chunk size for block processing
    };

    MetricsExtractionEngine(AudioFileLoader& loader,
        DataExport& exporter,
        Compressor& peakCompressor,
        Compressor& rmsCompressor,
        Metrics& metrics,
        juce::AudioProcessorValueTreeState& apvts,
        Config cfg);

    //==============================================================================
    
    /**
    * Triggers the metrics extaction process
    *
    * @param selectedFIle is the file selected by
    *        AudioFileLoader.chooseAudioFIle() invoked separately in PluginEditor.
    */
    void run(const juce::File& selectedFile);

    /**
    * Getter functions for UI progress
    */
    bool isProcessing() const noexcept { return processing.load(); }
    double getProgress() const noexcept { return progress.load(); }

private:
    /**
    * Prepares buffers for peak compressed signal and peak gain reduction,
    * rms compressed signal and rms gain reduction.
    * Compresses the full uncompressed audio signal.
    */
    void compressAudioFile();
    
    /**
    * Compresses the full audio signal in chunks.
    *
    * @param grBuffer the buffer for gain reduction signal.
    * @param audioBuffer the buffer for compressed signal.
    * @param isRMS the flag for peak or rms compression.
    * @param compressor the compressor object from PluginProcessor either for peak or rms compression.
    */
    void processBufferInChunks(juce::AudioBuffer<float>& grBuffer,
        juce::AudioBuffer<float>& audioBuffer,
        bool isRMS,
        Compressor& compressor);

    /**
    * Computes metrics for uncompressed, peak and rms compressed and gain reduction signal,
    */
    void getMetrics();
    
    /**
    * Build metrics report using computed metrics
    */
    juce::String buildMetricsReport() const;
    
    /**
    * Get parameter value
    */
    float getParam(const juce::String& id) const;
    
    /**
    * Formats the report of paramater configuration values
    */
    juce::String formatParameterBlock(const juce::String& title,
        const juce::String& prefix) const;

//==============================================================================
private:
    // Dependencies
    AudioFileLoader& loader;
    DataExport& exporter;
    Compressor& peakCompressor;
    Compressor& rmsCompressor;
    Metrics& metrics;
    juce::AudioProcessorValueTreeState& apvts;
    Config cfg;

    // FIle loading
    juce::File selectedFile;
    bool fileExists = false;
    double fileSampleRate = 0.0;

    // Buffers for metrics extraction
    juce::AudioBuffer<float> uncompressedSignal;
    juce::AudioBuffer<float> peakCompressedSignal;
    juce::AudioBuffer<float> peakGainReductionSignal;
    juce::AudioBuffer<float> rmsCompressedSignal;
    juce::AudioBuffer<float> rmsGainReductionSignal;

    // UI/progress
    std::atomic<bool> processing{ false };
    std::atomic<double> progress{ 0.0 };
};
