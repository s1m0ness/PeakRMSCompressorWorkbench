/*
 * Implementation based on the CTAGDRC project:
 * https://github.com/p-hlp/CTAGDRC
 *
 * The CTAGDRC project provides an open-source dynamic range compressor modeled
 * after the "ideal" VCA compressor circuit, as described in the paper
 * "Digital Dynamic Range Compressor Design – Tutorial and Analysis" by
 * Giannoulis, Massberg, & Reiss. This code draws on the design philosophy
 * and implementation patterns from CTAGDRC to handle compression workflows.
 *
 * The following sections integrate techniques and concepts derived from the
 * CTAGDRC project:
 * - Audio buffer management for compression (sidechain signal).
 * - Gain reduction and sidechain handling with detectors and gain computers.
 *
 * NOTE: This implementation reuses parts of the original CTAGDRC code.
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
#include "LevelDetector.h"
#include "GainComputer.h"
#include "../JuceLibraryCode/JuceHeader.h"

class Compressor
{
public:
    //==============================================================================
    Compressor() = default;
    ~Compressor();

    //==============================================================================
    void prepare(const juce::dsp::ProcessSpec& ps);

    //==============================================================================
    void setPower(bool);
    void setThreshold(float db);
    void setRatio(float db);
    void setAttack(float ms);
    void setRelease(float ms);
    void setKnee(float db);
    void setMakeup(float db);

    //==============================================================================
    float getMakeup();
    double getSampleRate();
    float getMaxGainReduction();
    juce::AudioBuffer<float> getGainReductionSignal();

    //==============================================================================
    void process(juce::AudioBuffer<float>& buffer, bool isRMSmode);

    /*
    * Applies Peak-Based Compression to the input buffer.
    *
    * Steps:
    * 1. Prepare the audio signal by calculating the raw sidechain signal.
    * 2. Use a peak detector to process the raw sidechain signal to calculate gain reduction.
    * 3. Apply gain reduction to the buffer based on the calculated sidechain signal.
    * 4. Optionally, store the gain reduction signal in a separate buffer for metrics extraction.
    * 5. Apply the makeup gain to restore the output level to the desired loudness.
    *
    * @param buffer       The input audio buffer to be compressed.
    * @param numSamples   The number of samples in the current audio buffer.
    * @param numChannels  The number of audio channels in the current audio buffer.
    * @param trackGR      Boolean flag to indicate whether to store the gain reduction signal for offline analysis.
    */
    void applyPeakCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, bool trackGR);

    /*
    * Applies RMS-Based Compression to the input buffer.
    *
    * Steps:
    * 1. Prepare the audio signal by calculating the raw sidechain signal.
    * 2. Use an RMS detector to process the raw sidechain signal and calculate gain reduction.
    * 3. Apply gain reduction to the buffer based on the calculated sidechain signal.
    * 4. Optionally, store the gain reduction signal in a separate buffer for metrics extraction.
    * 5. Apply the makeup gain to restore the output level to the desired loudness.
    *
    * @param buffer       The input audio buffer to be compressed.
    * @param numSamples   The number of samples in the current audio buffer.
    * @param numChannels  The number of audio channels in the current audio buffer.
    * @param trackGR      Boolean flag to indicate whether to store the gain reduction signal for offline analysis.
    */
    void applyRMSCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, bool trackGR);

    void prepareForMetricsExtraction(const juce::dsp::ProcessSpec& audioFilePs);
    void prepareForRealTimeProcessing();

private:
    //==============================================================================
    void setSidechainSignal(juce::AudioBuffer<float>& buffer, int numSamples);
    void applyCompressionToInputSignal(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, float makeup);

    void getGainReductionSignal(int numSamples, int numChannels);
   

    //Directly initialize process spec to avoid debugging problems
    juce::dsp::ProcessSpec procSpec{ -1, 0, 0 };

    juce::AudioBuffer<float> originalSignal;
    std::vector<float> sidechainSignal;
    float* rawSidechainSignal{ nullptr };

    std::vector<float> sidechainRight;

    // for offline metrics extraction
    juce::AudioBuffer<float> gainReductionSignal;

    LevelDetector levelDetector;
    
    GainComputer gainComputer;

    bool RMSModeEnabled{ false };
    bool bypassed{ false };
    
    float makeup{ 0.0f };
    float maxGainReduction{ 0.0f };
};