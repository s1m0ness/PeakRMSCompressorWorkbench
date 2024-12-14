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

#include "include/Compressor.h"
#include <sstream>

Compressor::~Compressor()
{
}
//==============================================================================
void Compressor::prepare(const juce::dsp::ProcessSpec& ps)
{
	procSpec = ps;
    levelDetector.prepare(ps.sampleRate);
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

// PEAK PARAMS
void Compressor::setThreshold(float thresholdInDb)
{
    gainComputer.setThreshold(thresholdInDb);
}

void Compressor::setRatio(float rat)
{
    gainComputer.setRatio(rat);
}

void Compressor::setAttack(float attackTimeInMs)
{
    levelDetector.setAttack(attackTimeInMs * 0.001);
}

void Compressor::setRelease(float releaseTimeInMs)
{
    levelDetector.setRelease(releaseTimeInMs * 0.001);
}

void Compressor::setKnee(float kneeInDb)
{
    gainComputer.setKnee(kneeInDb);
}

void Compressor::setMakeup(float makeupGainInDb)
{
    makeup = makeupGainInDb;
}


//==============================================================================
float Compressor::getMakeup()
{
    return makeup;
}

double Compressor::getSampleRate()
{
    return procSpec.sampleRate;
}

float Compressor::getMaxGainReduction()
{
    return maxGainReduction;
}

juce::AudioBuffer<float> Compressor::getGainReductionSignal()
{
    return gainReductionSignal;
}


// APPLY COMPRESSION
//==============================================================================
void Compressor::process(juce::AudioBuffer<float>& buffer, bool isRMSmode)
{
    if (!bypassed) {
        const auto numSamples = buffer.getNumSamples();
        const auto numChannels = buffer.getNumChannels();

        resetSizeSignals();

        jassert(numSamples == static_cast<int>(sidechainSignal.size()));

        if (!isRMSmode) {
            applyPeakCompression(buffer, numSamples, numChannels, false);
        } else {
            applyRMSCompression(buffer, numSamples, numChannels, false);
        }
    }
}

void Compressor::applyPeakCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, bool trackGR)
{
    setSidechainSignal(buffer, numSamples);

    // Compute attenuation - converts side-chain signal from linear to logarithmic domain
    gainComputer.applyCompressionToBuffer(rawSidechainSignal, numSamples); // Gain computer stage
    
    // Use smoothig detector filter for gain reduction - still logarithmic
    levelDetector.applyPeakDetector(rawSidechainSignal, numSamples); // Peak-based level detection stage 

    if (trackGR) { // for metrics extraction
        saveGainReductionSignal(numSamples, numChannels);
    }

    applyCompressionToInputSignal(buffer, numSamples, numChannels, getMakeup());
}

void Compressor::applyRMSCompression(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, bool trackGR)
{
    setSidechainSignal(buffer, numSamples);
    
    // Use smoothig detector filter for rms level computation in linear domain (before log conversion)
    levelDetector.applyRMSDetector(rawSidechainSignal, numSamples); // RMS-based level detection stage

    // Compute attenuation - converts side-chain signal from linear to logarithmic domain
    gainComputer.applyCompressionToBuffer(rawSidechainSignal, numSamples); // Gain computer stage

    if (trackGR) {
        saveGainReductionSignal(numSamples, numChannels);
    }

    applyCompressionToInputSignal(buffer, numSamples, numChannels, getMakeup());
}

// AUDIO BUFFERS HANDLING FOR COMPRESSION AND LOG->LIN CONVERTER
//==============================================================================
void Compressor::setSidechainSignal(juce::AudioBuffer<float>& buffer, int numSamples) {
    
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

void Compressor::applyCompressionToInputSignal(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels, float makeup)
{
    // Get minimum = max. gain reduction from side chain buffer
    maxGainReduction = juce::FloatVectorOperations::findMinimum(rawSidechainSignal, numSamples);

    // Add makeup gain and convert side-chain to linear domain
    for (int i = 0; i < numSamples; ++i) {
        sidechainSignal[i] = juce::Decibels::decibelsToGain(sidechainSignal[i] + makeup);
    }

    // Copy buffer to original signal
    for (int i = 0; i < numChannels; ++i) {
        originalSignal.copyFrom(i, 0, buffer, i, 0, numSamples);
    }

    // Multiply attenuation with buffer - apply compression
    for (int i = 0; i < numChannels; ++i) {
        juce::FloatVectorOperations::multiply(buffer.getWritePointer(i), rawSidechainSignal, buffer.getNumSamples());
    }
}

void Compressor::applyInputGain(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (prevInput == input) {
        buffer.applyGain(0, numSamples, juce::Decibels::decibelsToGain(prevInput));
    } else {
        buffer.applyGainRamp(0, numSamples, juce::Decibels::decibelsToGain(prevInput), juce::Decibels::decibelsToGain(input));
        prevInput = input;
    }
}

// ADDITIONAL FUNCTIONS
//==============================================================================
void Compressor::saveGainReductionSignal(int numSamples, int numChannels)
{
    gainReductionSignal.setSize(numChannels, numSamples, false, true, true);
    for (int channel = 0; channel < numChannels; ++channel) {
        for (int sample = 0; sample < numSamples; sample++) {
            gainReductionSignal.setSample(channel, sample, juce::Decibels::decibelsToGain(rawSidechainSignal[sample]));
        }
    }
}

// This is called during the metrics extraction process
// when the entire audio gets compressed in chunks
void Compressor::resizeSignals(int chunkSize)
{
    sidechainSignal.resize(chunkSize, 0.0f);
    originalSignal.setSize(2, chunkSize);
    rawSidechainSignal = sidechainSignal.data(); 
}

// Resize to original dimensions
void Compressor::resetSizeSignals()
{
    sidechainSignal.resize(procSpec.maximumBlockSize, 0.0f); 
    originalSignal.setSize(2, procSpec.maximumBlockSize);
    rawSidechainSignal = sidechainSignal.data();
}
