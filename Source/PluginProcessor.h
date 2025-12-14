/*
 * Implementation based on the JUCE framework.
 * This file contains the core audio processing logic for the plugin.
 *
 * Key Features:
 * - Handles audio processing, including compression.
 * - Implements RMS and Peak-based dynamic range compression using the JUCE DSP framework.
 * - Provides support for real-time and offline audio analysis.
 * - Saves compressed audio outputs and extracted metrics to user-defined locations.
 *
 * Additional Features:
 * - Extends the basic JUCE framework to include metrics extraction functionality, enabling analysis
 *   of dynamic range compression applied on audio signal, using peak-based or RMS-based level detection.
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

// For dynamic range compression
#include <../Source/dsp/include/Compressor.h>

// For metering
#include <../Source/dsp/include/LevelEnvelopeFollower.h>

// For metrics extraction
#include <../Source/metrics/include/AudioFileLoader.h>
#include <../Source/metrics/include/MetricsExtractionEngine.h>
#include <../Source/metrics/include/DataExport.h>
#include <../Source/metrics/include/Metrics.h>

// Constants, presets and config
#include <../Source/util/include/Constants.h>
#include <../Source/util/include/Presets.h>
#include <../Source/util/include/Config.h>

//==============================================================================
/**
*/
class PeakRMSCompressorWorkbenchAudioProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    PeakRMSCompressorWorkbenchAudioProcessor();
    ~PeakRMSCompressorWorkbenchAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    /**
    * Updates the compressor parameters based on the selected compression mode.
    * Adjusts either Peak or RMS parameters depending on the isRMSMode flag.
    *
    * @param isRMSMode True for RMS mode, false for Peak mode.
    */
    void updateCompressionMode(bool);


    // PARAMETERS HANDLING
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // Getter for acquiring preset parameter values 
    float getParameterValue(const juce::String& paramID) const {
        if (auto* param = parameters.getRawParameterValue(paramID)) {
            return param->load();
        }
        jassertfalse; // Invalid parameter ID
        return 0.0f;
    }

    // APPLY PRESET
    //==============================================================================
    /**
    * Updates the compressor parameters based on the selected preset.
    *
    * @param presetId for chosen preset.
    */
    void applyPreset(int presetId);

    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> gainReduction;
    std::atomic<float> currentInput;
    std::atomic<float> currentOutput;

    Compressor peakCompressor;
    Compressor rmsCompressor;

    bool isRMSMode{ false };
    bool isMuted{ false };

    std::map<int, PresetStruct> createPresetParameters();
    std::map<int, PresetStruct> PresetParameters;

    MetricsExtractionEngine& getMetricsExtractionEngine() { 
        return metricsExtractionEngine;
    }
    
    AudioFileLoader& getAudioFileLoader() {
        return audioFileLoader;
    }

private:
    //==============================================================================
    LevelEnvelopeFollower inLevelFollower;
    LevelEnvelopeFollower outLevelFollower;

    juce::AudioFormatManager formatManager; // Handles audio format readers


    //==============================================================================
    juce::File outputDirectory = juce::File(Config::OutputPath::path);

    juce::AudioBuffer<float> peakCompressedSignal,
        rmsCompressedSignal,
        rmsGainReductionSignal,
        peakGainReductionSignal;
    Metrics metrics;

    // Metrics extraction pipeline
    AudioFileLoader audioFileLoader;
    DataExport dataExport;
    MetricsExtractionEngine metricsExtractionEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeakRMSCompressorWorkbenchAudioProcessor)
};
