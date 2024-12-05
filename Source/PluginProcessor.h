/*
 * Implementation based on the JUCE framework.
 * This file contains the core audio processing logic for the plugin.
 *
 * Key Features:
 * - Handles audio processing, including compression.
 * - Implements RMS and Peak-based dynamic range compression using the JUCE DSP framework.
 * - Provides support for real-time and offline audio analysis.
 * - Saves compressed audio outputs to user-defined locations.
 *
 * Additional Features:
 * - Extends the basic JUCE framework to include metrics extraction functionality, enabling dynamic analysis of audio signals.
 *
 * NOTE: This implementation integrates with the JUCE AudioProcessor API and follows its standard plugin structure.
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
#include <../Source/dsp/include/Compressor.h>
#include <../Source/dsp/include/LevelEnvelopeFollower.h>
#include <../Source/metrics/include/Metrics.h>
#include <../Source/util/include/Constants.h>


//==============================================================================
/**
*/
class PeakRMSCompressorWorkbenchAudioProcessor  : public juce::AudioProcessor, 
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    PeakRMSCompressorWorkbenchAudioProcessor();
    ~PeakRMSCompressorWorkbenchAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // PARAMETERS HANDLING
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    /**
    * Updates the compressor parameters based on the selected compression mode.
    * Adjusts either Peak or RMS parameters depending on the isRMSMode flag.
    *
    * @param isRMSMode True for RMS mode, false for Peak mode.
    */
    void updateCompressionMode(bool);

    /**
    * Updates the compressor parameters based on the selected preset.
    *
    * @param presetId for chosen preset.
    */
    void applyPreset(int presetId);  

    // METRICS EXTRACTION
    //==============================================================================
    /**
    * Extracts metrics from the audio file.
    * Compresses the audio, calculates metrics, and saves outputs.
    */
    void extractMetrics();

    /**
    * Loads user selected audio file.
    */
    void loadFile();

    /**
    * Saves selected audio file into the fullAudioBuffer.
    */
    void saveFileToFullBuffer();

    /**
    * Compresses the full audio buffer using both Peak and RMS compression.
    * Outputs compressed audio and gain reduction signals.
    */
    void compressFullBuffer();

    /**
    * Processes the audio buffer in smaller chunks.
    * Handles Peak and RMS compression based on the isRMS flag.
    *
    * @param grBuffer Gain reduction buffer to store results.
    * @param buffer Input/output audio buffer for processing.
    * @param chunkSize Number of samples to process per chunk.
    * @param isRMS Flag to specify whether RMS compression is applied.
    */
    void processBufferInChunks(juce::AudioBuffer<float>& grBuffer, juce::AudioBuffer<float>& buffer, int chunkSize, bool isRMS);

    /**
     * Saves the compressed audio outputs and metrics.
     * Writes Peak and RMS compressed audio files and metrics summary.
     */
    void saveOutputs();

    /**
    * Saves an audio buffer to a .wav file.
    * Handles file creation, writing, and deletion of existing files.
    *
    * @param buffer The audio buffer to save.
    * @param outputFile File path to save the audio buffer.
    * @param sampleRate Sample rate of the audio buffer.
    */
    void saveAudioBufferToFile(const juce::AudioBuffer<float>& buffer, const juce::File& outputFile, double sampleRate);

    /**
    * Saves metrics content to a .txt file.
    * Handles file creation, directory validation, and overwriting.
    *
    * @param metricsContent Metrics data as a formatted string.
    * @param destinationPath Path to save the metrics file.
    */
    void saveMetricsToFile(const juce::String& metricsContent, const juce::String& destinationPath);

    //==============================================================================
    bool getIsBufferReady() const { return isBufferReady; }
   
    //==============================================================================
    std::atomic<float> gainReduction;
    std::atomic<float> currentInput;
    std::atomic<float> currentOutput;

    Compressor compressor;

    bool isMuted{ false };

    std::atomic<bool> isProcessing{ false };
    std::thread processingThread; // Thread for background processing
    bool fileExists = false;

    double progress{ 0.0 }; // Thread-safe progress value

    public:
        float getParameterValue(const juce::String& paramID) const {
            if (auto* param = parameters.getRawParameterValue(paramID)) {
                return param->load();
            }
            jassertfalse; // Invalid parameter ID
            return 0.0f;
        }

private:
    juce::AudioProcessorValueTreeState parameters;
    //==============================================================================
    LevelEnvelopeFollower inLevelFollower;
    LevelEnvelopeFollower outLevelFollower;
    juce::AudioFormatManager formatManager; // Handles audio format readers
    juce::AudioBuffer<float> fullAudioBuffer; 
    juce::File selectedFile;
    juce::String audioFilePath;
    juce::AudioBuffer<float> uncompressedInputAudio, 
                             peakCompressedOutputAudio, 
                             rmsCompressedOutputAudio, 
                             rmsGainReductionSignal,
                             peakGainReductionSignal;
    bool isBufferReady = false;
    Metrics metrics;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakRMSCompressorWorkbenchAudioProcessor)
};
