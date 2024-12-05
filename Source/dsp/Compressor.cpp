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
 * - Audio buffer management for compression.
 * - Gain reduction and sidechain handling with detectors and gain computers.
 * 
 * NOTE: This implementation directly reuses parts of the original CTAGDRC code without modification.
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

#include "../Source/dsp/include/Compressor.h"
#include <sstream>

Compressor::~Compressor()
{
}
//==============================================================================
void Compressor::prepare(const juce::dsp::ProcessSpec& ps)
{
	procSpec = ps;
    peakDetector.prepare(ps.sampleRate);
    rmsDetector.prepare(ps.sampleRate);
	originalSignal.setSize(2, ps.maximumBlockSize);
	sidechainSignal.resize(ps.maximumBlockSize, 0.0f);
	rawSidechainSignal = sidechainSignal.data();
	originalSignal.clear();
}

//==============================================================================
void Compressor::setPower(bool newPower)
{
    bypassed = newPower;
}

void Compressor::setRMSMode(bool newMode)
{
    RMSModeEnabled = newMode;
}

void Compressor::setPeakThreshold(float thresholdInDb)
{
    peakGainComputer.setThreshold(thresholdInDb);
}

void Compressor::setPeakRatio(float rat)
{
    peakGainComputer.setRatio(rat);
}

void Compressor::setPeakAttack(float attackTimeInMs)
{
    peakDetector.setAttack(attackTimeInMs * 0.001);
}

void Compressor::setPeakRelease(float releaseTimeInMs)
{
    peakDetector.setRelease(releaseTimeInMs * 0.001);
}

void Compressor::setPeakKnee(float kneeInDb)
{
    peakGainComputer.setKnee(kneeInDb);
}

void Compressor::setPeakMakeup(float makeupGainInDb)
{
    peakMakeup = makeupGainInDb;
}

void Compressor::setRMSThreshold(float thresholdInDb)
{
    rmsGainComputer.setThreshold(thresholdInDb);
}

void Compressor::setRMSRatio(float rat)
{
    rmsGainComputer.setRatio(rat);
}

void Compressor::setRMSAttack(float attackTimeInMs)
{
    rmsDetector.setAttack(attackTimeInMs * 0.001);
}

void Compressor::setRMSRelease(float releaseTimeInMs)
{
    rmsDetector.setRelease(releaseTimeInMs * 0.001);
}

void Compressor::setRMSKnee(float kneeInDb)
{
    rmsGainComputer.setKnee(kneeInDb);
}

void Compressor::setRMSMakeup(float makeupGainInDb)
{
    rmsMakeup = makeupGainInDb;
}

//==============================================================================
float Compressor::getPeakMakeup()
{
    return peakMakeup;
}

float Compressor::getRMSMakeup()
{
    return rmsMakeup;
}

double Compressor::getSampleRate()
{
    return procSpec.sampleRate;
}

float Compressor::getMaxGainReduction()
{
    return maxGainReduction;
}

juce::AudioBuffer<float> Compressor::getPeakGainReductionSignal()
{
    return peakGainReduction;
}

juce::AudioBuffer<float> Compressor::getRMSGainReductionSignal()
{
    return rmsGainReduction;
}

// APPLY COMPRESSION
//==============================================================================
void Compressor::process(juce::AudioBuffer<float>& buffer)
{
    if (!bypassed)
    {
        const auto numSamples = buffer.getNumSamples();
        const auto numChannels = buffer.getNumChannels();

        ResetSizeSignals();

        jassert(numSamples == static_cast<int>(sidechainSignal.size()));

        if (!RMSModeEnabled)
        {
            applyPeakCompression(buffer, numSamples, numChannels, false);
        }
        else
        {
            applyRMSCompression(buffer, numSamples, numChannels, false);
        }
    }
}

void Compressor::applyPeakCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, bool trackGR)
{
    prepareCompression(buffer, numSamples);

    // Compute attenuation - converts side-chain signal from linear to logarithmic domain
    peakGainComputer.applyCompressionToBuffer(rawSidechainSignal, numSamples); // Gain computer stage
    
    // Smooth attenuation - still logarithmic
    peakDetector.applyPeakDetector(rawSidechainSignal, numSamples); // Peak-based level detection stage 

    if (trackGR)
    {
        peakGainReduction.setSize(numChannels, numSamples, false, true, true);
        for (int i = 0; i < numSamples; ++i)
        {
            peakGainReduction.setSample(0, i, juce::Decibels::decibelsToGain(rawSidechainSignal[i]));
            /*DBG(peakGainReduction.getSample(0, i));*/
        }
    }

    finishCompression(buffer, numSamples, numChannels, getPeakMakeup());
}

void Compressor::applyRMSCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, bool trackGR)
{
    prepareCompression(buffer, numSamples);
    rmsDetector.applyRMSDetector(rawSidechainSignal, numSamples); // RMS-based level detection stage
    rmsGainComputer.applyCompressionToBuffer(rawSidechainSignal, numSamples); // Gain computer stage

    if (trackGR)
    {
        rmsGainReduction.setSize(numChannels, numSamples, false, true, true);
        for (int i = 0; i < numSamples; ++i)
        {
            rmsGainReduction.setSample(0, i, juce::Decibels::decibelsToGain(rawSidechainSignal[i]));
        }
    }

    finishCompression(buffer, numSamples, numChannels, getRMSMakeup());
}

// AUDIO BUFFERS HANDLING FOR COMPRESSION AND LOG->LIN, LIN->LOG CONVERTER
//==============================================================================
void Compressor::prepareCompression(juce::AudioBuffer<float>& buffer, int numSamples) {
    
    // Clear any old samples
    originalSignal.clear();
    juce::FloatVectorOperations::fill(rawSidechainSignal, 0.0f, numSamples);
    maxGainReduction = 0.0f;

    // Apply input gain
    applyInputGain(buffer, numSamples);

    // Get max l/r amplitude values and fill sidechain signal
    juce::FloatVectorOperations::abs(rawSidechainSignal, buffer.getReadPointer(0), numSamples);
    juce::FloatVectorOperations::max(rawSidechainSignal, rawSidechainSignal, buffer.getReadPointer(1), numSamples);

}

void Compressor::finishCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, float makeup)
{
    // Get minimum = max. gain reduction from side chain buffer
    maxGainReduction = juce::FloatVectorOperations::findMinimum(rawSidechainSignal, numSamples);

    // Add makeup gain and convert side-chain to linear domain
    for (int i = 0; i < numSamples; ++i) {
        sidechainSignal[i] = juce::Decibels::decibelsToGain(sidechainSignal[i] + makeup);
    }

    // Copy buffer to original signal
    for (int i = 0; i < numChannels; ++i)
        originalSignal.copyFrom(i, 0, buffer, i, 0, numSamples);

    // Multiply attenuation with buffer - apply compression
    for (int i = 0; i < numChannels; ++i)
        juce::FloatVectorOperations::multiply(buffer.getWritePointer(i), rawSidechainSignal, buffer.getNumSamples());
}

inline void Compressor::applyInputGain(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (prevInput == input)
        buffer.applyGain(0, numSamples, juce::Decibels::decibelsToGain(prevInput));
    else
    {
        buffer.applyGainRamp(0, numSamples, juce::Decibels::decibelsToGain(prevInput), juce::Decibels::decibelsToGain(input));
        prevInput = input;
    }
}

//==============================================================================
void Compressor::ResizeSignals(int numSamples)
{
    sidechainSignal.resize(numSamples, 0.0f);
    originalSignal.setSize(2, numSamples);
    rawSidechainSignal = sidechainSignal.data(); 
}

void Compressor::ResetSizeSignals()
{
    sidechainSignal.resize(procSpec.maximumBlockSize, 0.0f); 
    originalSignal.setSize(2, procSpec.maximumBlockSize);
    rawSidechainSignal = sidechainSignal.data();
}