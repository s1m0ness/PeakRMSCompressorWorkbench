/*
 * This file defines the Metrics class, responsible for calculating and storing audio analysis metrics.
 *
 * Key Features:
 * - Calculates essential audio metrics including RMSE, correlation, crest factor, and gain reduction.
 * - Computes advanced metrics such as Loudness Range (LRA) using EBU R128 standards.
 * - Supports both RMS and Peak-based compression analysis.
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

#include "include/Metrics.h"
#include <include_juce_dsp.cpp>
#include <cmath>
#include <numeric>

//==============================================================================
Metrics::Metrics() = default;

Metrics::~Metrics() = default;

//==============================================================================
void Metrics::prepare(const double& fs)
{
    sampleRate = fs;
}

//==============================================================================
void Metrics::setUncompressedSignal(juce::AudioBuffer<float>* signal)
{
    uncompressedMetrics.signal = signal;
    uncompressedMetrics.signalName = "Uncompressed signal";
    uncompressedMetrics.isCompressed = false;
}

void Metrics::setPeakCompressedSignal(juce::AudioBuffer<float>* signal)
{
    peakMetrics.signal = signal;
    peakMetrics.signalName = "Peak compressed signal";
    peakMetrics.isCompressed = true;
}

void Metrics::setRMSCompressedSignal(juce::AudioBuffer<float>* signal)
{
    rmsMetrics.signal = signal;
    rmsMetrics.signalName = "RMS compressed signal";
    rmsMetrics.isCompressed = true;
}

void Metrics::setPeakGainReductionSignal(juce::AudioBuffer<float>* signal)
{
    peakMetrics.GRSignal = signal;
}

void Metrics::setRMSGainReductionSignal(juce::AudioBuffer<float>* signal)
{
    rmsMetrics.GRSignal = signal;
}

//==============================================================================
const Metrics::CompressionMetrics& Metrics::getUncompressedMetrics() const
{
    return uncompressedMetrics;
}

const Metrics::CompressionMetrics& Metrics::getPeakMetrics() const
{
    return peakMetrics;
}

const Metrics::CompressionMetrics& Metrics::getRMSMetrics() const
{
    return rmsMetrics;
}

//==============================================================================
void Metrics::extractMetrics(float peakMakeup, float rmsMakeup)
{
    if (!validateSignals()) {
        return;
    }

    auto getAndSaveMetrics = [this](CompressionMetrics& metrics, float makeup) {
        metrics.makeup = makeup;
        metrics.meanEnergy = getAverageEnergy(*metrics.signal);
        metrics.peak = getPeakValue(*metrics.signal);
        metrics.rms = getRMSValue(metrics.meanEnergy);
        metrics.crestFactor = getCrestFactor(metrics.peak, metrics.rms);
        metrics.lufs = getLUFS(*metrics.signal);
        metrics.lra = getLRA(*metrics.signal);

        if (metrics.isCompressed) {
            metrics.dynamicRangeReductionCrest = getDynamicRangeReductionCrest(metrics.crestFactor);
            metrics.dynamicRangeReductionLUFS = getDynamicRangeReductionLUFS(metrics.lufs);
            metrics.dynamicRangeReductionLRA = getDynamicRangeReductionLRA(metrics.lra);
            metrics.transientImpact = getTransientImpact(*metrics.signal);
            metrics.transientEnergyPreservation = getTransientEnergyPreservation(*metrics.signal);
            metrics.harmonicDistortion = getHarmonicDistortion(*metrics.signal);

            metrics.avgGR = getAverageGainReduction(*metrics.GRSignal);
            metrics.maxGR = getMaxGainReduction(*metrics.GRSignal);
            metrics.stdDevGR = getStdDevGainReduction(*metrics.GRSignal, metrics.avgGR);
            metrics.energyGR = getEnergyGainReduction(*metrics.GRSignal);
            metrics.rateOfChangeGR = getRateOfChangeGainReduction(*metrics.GRSignal);
            metrics.compressionActivityRatio = getCompressionActivityRatio(*metrics.GRSignal);
        }
    };

    getAndSaveMetrics(uncompressedMetrics, 0.0f);
    getAndSaveMetrics(peakMetrics, peakMakeup);
    getAndSaveMetrics(rmsMetrics, rmsMakeup);
}

// Signal metrics
//==============================================================================
float Metrics::getPeakValue(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    int totalSamples = buffer.getNumSamples();
    int totalChannels = buffer.getNumChannels();

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            peak = std::max(peak, std::abs(channelData[sample])); // Use absolute value here.
        }
    }
    return peak; // in linear gain
}

float Metrics::getAverageEnergy(const juce::AudioBuffer<float>& buffer)
{
    float meanSquare = 0.0f;
    int totalSamples = buffer.getNumSamples();
    int totalChannels = buffer.getNumChannels();

    const float silenceThreshold = 0.0001f;

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            if (std::abs(channelData[sample]) >= silenceThreshold) {
                meanSquare += channelData[sample] * channelData[sample];
            }
        }
    }
    return meanSquare / static_cast<float>(totalSamples * totalChannels); // in linear gain
}

float Metrics::getRMSValue(float meanSquare) 
{
    return std::sqrtf(meanSquare); // in linear gain
}

float Metrics::getCrestFactor(float peakValue, float rmsValue) 
{
    return juce::Decibels::gainToDecibels(peakValue / rmsValue); // in db
}

float Metrics::getLUFS(const juce::AudioBuffer<float>& buffer)
{
    // Copy the buffer and apply K-weighting
    juce::AudioBuffer<float> kWeightedBuffer;
    kWeightedBuffer.makeCopyOf(buffer);
    applyKWeighting(kWeightedBuffer);

    return getIntegratedLoudness(getAverageEnergy(kWeightedBuffer)); // in db
}

float Metrics::getLRA(const juce::AudioBuffer<float>& buffer)
{
    // Copy and apply K-weighting filters
    juce::AudioBuffer<float> kWeightedBuffer;
    kWeightedBuffer.makeCopyOf(buffer);
    applyKWeighting(kWeightedBuffer);

    std::vector<float> shortTermLoudness = getShortTermLoudness(kWeightedBuffer);
    
    // Step 2: Calculate LRA from short-term loudness
    if (shortTermLoudness.empty())
        return 0.0f;

    // Sort loudness values to calculate percentiles
    std::sort(shortTermLoudness.begin(), shortTermLoudness.end());

    // Calculate 10th and 95th percentiles
    size_t n = shortTermLoudness.size();
    float lowPercentile = shortTermLoudness[static_cast<size_t>(0.1 * n)];
    float highPercentile = shortTermLoudness[static_cast<size_t>(0.95 * n)];

    // LRA is defined as the difference between the high and low percentiles
    return highPercentile - lowPercentile; // in db
}

float Metrics::getDynamicRangeReductionCrest(float compressedCrestFactor) {
    return uncompressedMetrics.crestFactor - compressedCrestFactor;  // in db
}

float Metrics::getDynamicRangeReductionLUFS(float compressedLUFS) {
    return uncompressedMetrics.lufs - compressedLUFS; // in db
}

float Metrics::getDynamicRangeReductionLRA(float compressedLRA) {
    return uncompressedMetrics.lra - compressedLRA; // in db
}

float Metrics::getTransientImpact(const juce::AudioBuffer<float>& compressedBuffer)
{
    // Helper: compute a simple transient strength measure
    auto computeTransientStrength = [](const juce::AudioBuffer<float>& buffer)
        {
            float maxDelta = 0.0f;
            const int totalSamples = buffer.getNumSamples();
            const int totalChannels = buffer.getNumChannels();

            for (int channel = 0; channel < totalChannels; ++channel)
            {
                const float* channelData = buffer.getReadPointer(channel);

                for (int sample = 1; sample < totalSamples; ++sample)
                {
                    const float delta = std::abs(channelData[sample] - channelData[sample - 1]);
                    if (delta > maxDelta)
                        maxDelta = delta;
                }
            }

            return maxDelta; // linear, 0..1-ish
        };

    const float uncompressedTransients = computeTransientStrength(*uncompressedMetrics.signal);
    const float compressedTransients = computeTransientStrength(compressedBuffer);

    if (uncompressedTransients <= 0.0f)
        return 0.0f; // no transient content to compare

    // Fraction of lost transient strength: 0 = no loss, 1 = fully flattened
    float impact = (uncompressedTransients - compressedTransients) / uncompressedTransients;

    // Clamp to [0, 1] in case of weird numerical stuff
    impact = juce::jlimit(0.0f, 1.0f, impact);

    return impact;
}


float Metrics::getTransientEnergyPreservation(const juce::AudioBuffer<float>& compressedBuffer) 
{
    const float transientThresholdMultiplier = 3.0f; // Multiplier for transient threshold

    std::vector<juce::AudioBuffer<float>> uncompressedWindows = getWindowsFromBuffer(*uncompressedMetrics.signal);
    std::vector<juce::AudioBuffer<float>> compressedWindows = getWindowsFromBuffer(compressedBuffer);

    std::vector<float> uncompressedShortTermRMSvalues, compressedShortTermRMSvalues;

    for (const auto& windowBuffer : uncompressedWindows) {
        uncompressedShortTermRMSvalues.push_back(juce::Decibels::gainToDecibels(getRMSValue(getAverageEnergy(windowBuffer))));
    }

    for (const auto& windowBuffer : compressedWindows) {
        compressedShortTermRMSvalues.push_back(juce::Decibels::gainToDecibels(getRMSValue(getAverageEnergy(windowBuffer))));
    }

    float uncompressedMeanRMS = std::accumulate(uncompressedShortTermRMSvalues.begin(),
        uncompressedShortTermRMSvalues.end(), 0.0f)
        / uncompressedShortTermRMSvalues.size();
    const float threshold = transientThresholdMultiplier * uncompressedMeanRMS;

    float uncompressedEnergy = 0.0f;
    float compressedEnergy = 0.0f;

    for (size_t i = 0; i < uncompressedWindows.size(); ++i) {
        float rmsUncompressed = uncompressedShortTermRMSvalues[i];
        if (rmsUncompressed > threshold) {
            float rmsCompressed = compressedShortTermRMSvalues[i];
            uncompressedEnergy += rmsUncompressed * rmsUncompressed; 
            compressedEnergy += rmsCompressed * rmsCompressed;       
        }
    }
    return (uncompressedEnergy > 0.0f) ? std::min(compressedEnergy / uncompressedEnergy, 1.0f) : 0.0f;
}

float Metrics::getHarmonicDistortion(const juce::AudioBuffer<float>& compressedBuffer)
{
    const int totalSamples = compressedBuffer.getNumSamples();
    const int totalChannels = compressedBuffer.getNumChannels();
    float errorSum = 0.0f;

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* originalData = uncompressedMetrics.signal->getReadPointer(channel);
        const float* compressedData = compressedBuffer.getReadPointer(channel);
        for (int i = 0; i < totalSamples; ++i) {
            float error = originalData[i] - compressedData[i];
            errorSum += error * error;
        }
    }
    float errorRMS = getRMSValue(errorSum / (totalSamples * totalChannels));

    return (uncompressedMetrics.rms > 0.0f) ? (errorRMS / uncompressedMetrics.rms) : -100.0f;
}

// Gain reduction signal metrics
//==============================================================================
float Metrics::getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    float maxReduction = 0;
    int totalSamples = gainReductionBuffer. getNumSamples();
    int totalChannels = gainReductionBuffer.getNumChannels();

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            float reduction = juce::Decibels::gainToDecibels(channelData[sample]);
            if (reduction == -100) { continue; } // ignore silence
            maxReduction = std::min(maxReduction, reduction); // Track max = min reduction
        }
    }
    return std::abs(maxReduction); // in db
}

float Metrics::getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    float totalGainReduction = 0.0f;
    int totalSamples = gainReductionBuffer.getNumSamples();
    int totalChannels = gainReductionBuffer.getNumChannels();

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            float reduction = juce::Decibels::gainToDecibels(channelData[sample]);
            if (reduction == -100) { continue; } // ignore silence
            totalGainReduction += std::fabs(reduction);   // Sum absolute reductions
        }
    }
    return totalGainReduction / (totalSamples * totalChannels); // in db
}

float Metrics::getStdDevGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer, float mean)
{
    float sum = 0.0f;
    int totalSamples = gainReductionBuffer.getNumSamples();
    int totalChannels = gainReductionBuffer.getNumChannels();

    // Step 2: Compute the sum of squared deviations from the mean
    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            float deviation = channelData[sample] - mean;
            sum += deviation * deviation;
        }
    }
    return std::sqrt(sum / (totalSamples * totalChannels));
}

float Metrics::getEnergyGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    return std::abs(juce::Decibels::gainToDecibels(getAverageEnergy(gainReductionBuffer))); // in db
}

float Metrics::getRateOfChangeGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    float sumRateOfChange = 0.0f;
    int totalSamples = gainReductionBuffer.getNumSamples();
    int totalChannels = gainReductionBuffer.getNumChannels();

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 1; sample < totalSamples; ++sample) {
            sumRateOfChange += std::abs(channelData[sample] - channelData[sample - 1]);
        }
    }
    return (sumRateOfChange / ((totalSamples - 1) * totalChannels)) * sampleRate;
}

float Metrics::getCompressionActivityRatio(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    int activeSamples = 0;
    int totalSamples = gainReductionBuffer.getNumSamples();
    int totalChannels = gainReductionBuffer.getNumChannels();

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            if (juce::Decibels::gainToDecibels(channelData[sample]) < 0) {
                ++activeSamples;
            }
        }
    }
    return static_cast<float>(activeSamples) / (totalSamples * totalChannels);
}

// Signal handling and modifications
//==============================================================================
bool Metrics::validateSignals() const
{
    if (!uncompressedMetrics.signal ||
        !peakMetrics.signal ||
        !rmsMetrics.signal ||
        !peakMetrics.GRSignal ||
        !rmsMetrics.GRSignal) {
        return false;
    }

    if (uncompressedMetrics.signal->getNumSamples() == 0 ||
        peakMetrics.signal->getNumSamples() == 0 ||
        rmsMetrics.signal->getNumSamples() == 0 ||
        peakMetrics.GRSignal->getNumSamples() == 0 ||
        rmsMetrics.GRSignal->getNumSamples() == 0) {
        return false;
    }

    bool sameNumOfChannels =
        uncompressedMetrics.signal->getNumChannels() == peakMetrics.signal->getNumChannels() &&
        peakMetrics.signal->getNumChannels() == rmsMetrics.signal->getNumChannels() &&
        rmsMetrics.signal->getNumChannels() == peakMetrics.GRSignal->getNumChannels() &&
        peakMetrics.GRSignal->getNumChannels() == rmsMetrics.GRSignal->getNumChannels();

    bool sameNumOfSamples =
        uncompressedMetrics.signal->getNumSamples() == peakMetrics.signal->getNumSamples() &&
        peakMetrics.signal->getNumSamples() == rmsMetrics.signal->getNumSamples() &&
        rmsMetrics.signal->getNumSamples() == peakMetrics.GRSignal->getNumSamples() &&
        peakMetrics.GRSignal->getNumSamples() == rmsMetrics.GRSignal->getNumSamples();

    return sameNumOfChannels && sameNumOfSamples;
}

// Modifies input buffer for LUFS and LRA calculations
void Metrics::applyKWeighting(juce::AudioBuffer<float>& buffer)
{
    // Create the K-weighting filters
    juce::dsp::IIR::Filter<float> lowShelfFilter, highShelfFilter;

    // Configure filter coefficients
    lowShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, 1681.974450955533, 0.7071752369554196, 1.53512485958697);
    highShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 424.318677406412, 0.7071752369554196, 1.0);

    // Convert the buffer into a JUCE AudioBlock
    juce::dsp::AudioBlock<float> audioBlock(buffer);

    for (size_t channel = 0; channel < static_cast<size_t>(audioBlock.getNumChannels()); ++channel) {
        auto singleChannelBlock = audioBlock.getSingleChannelBlock(channel);
        juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
        
        // Apply the filters
        lowShelfFilter.process(context);  // Apply low-shelf filter
        highShelfFilter.process(context); // Apply high-shelf filter
    }
}

// For short-term calculations
std::vector<juce::AudioBuffer<float>> Metrics::getWindowsFromBuffer(const juce::AudioBuffer<float>& buffer)
{
    const int totalSamples = buffer.getNumSamples();
    const int totalChannels = buffer.getNumChannels();
    const int windowSize = static_cast<int>(windowDuration * sampleRate);
    const int hopSize = static_cast<int>(hopDuration * sampleRate);

    std::vector<juce::AudioBuffer<float>> windowsBuffers;

    for (int startSample = 0; startSample + windowSize <= totalSamples; startSample += hopSize) {
        juce::AudioBuffer<float> windowBuffer(totalChannels, windowSize);
        for (int channel = 0; channel < totalChannels; ++channel) {
            const float* source = buffer.getReadPointer(channel, startSample);
            float* dest = windowBuffer.getWritePointer(channel);
            std::copy(source, source + windowSize, dest);
        }
        windowsBuffers.push_back(std::move(windowBuffer));
    }
    return windowsBuffers;
}

// Additional computation for LUFS and LRA
//==============================================================================
float Metrics::getIntegratedLoudness(float meanEnergy)
{
    return juce::Decibels::gainToDecibels(meanEnergy) - 0.691f; // K-weighting correction, in db
}

std::vector<float> Metrics::getShortTermLoudness(juce::AudioBuffer<float>& buffer)
{
    // Step 1: Extract all windows from the buffer
    std::vector<juce::AudioBuffer<float>> windowBuffers = getWindowsFromBuffer(buffer);
    std::vector<float> shortTermLoudness;

    // Step 2: Iterate over each window and calculate loudness
    for (const auto& windowBuffer : windowBuffers) {
        float shortTermloudness = getIntegratedLoudness(getAverageEnergy(windowBuffer));
        shortTermLoudness.push_back(shortTermloudness);
    }
    return shortTermLoudness; // in db
}
