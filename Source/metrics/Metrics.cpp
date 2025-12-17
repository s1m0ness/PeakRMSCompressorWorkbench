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

#pragma once
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
void Metrics::extractMetrics()
{
    if (!validateSignals()) {
        return;
    }

    auto computeMetrics = [this](CompressionMetrics& metrics) {

        
        // 1. Signal instensity and dynamic range metrics
        metrics.meanEnergy = getAverageEnergy(*metrics.signal);
        metrics.peak = getPeakValue(*metrics.signal);
        metrics.rms = getRMSValue(metrics.meanEnergy);
        metrics.crestFactor = getCrestFactor(metrics.peak, metrics.rms);
        metrics.lufs = getLUFS(*metrics.signal);
        metrics.lra = getLRA(*metrics.signal);

        if (metrics.isCompressed) {
            // 2. Compression impact metrics
            metrics.dynamicRangeReductionCrest = getDynamicRangeReductionCrest(metrics.crestFactor);
            metrics.dynamicRangeReductionLRA = getDynamicRangeReductionLRA(metrics.lra);
            metrics.transientImpact = getTransientImpact(*metrics.signal);
            metrics.transientEnergyPreservation = getTransientEnergyPreservation(*metrics.signal);
            metrics.harmonicDistortion = getWaveformDistortion(*metrics.signal);

            // 3. Gain reduction metrics
            metrics.avgGR = getAverageGainReduction(*metrics.GRSignal);
            metrics.maxGR = getMaxGainReduction(*metrics.GRSignal);
            metrics.stdDevGR = getStdDevGainReduction(*metrics.GRSignal, metrics.avgGR);
            metrics.energyGR = getEnergyGainReduction(*metrics.GRSignal);
            metrics.rateOfChangeGR = getRateOfChangeGainReduction(*metrics.GRSignal);
            metrics.compressionActivityRatio = getCompressionActivityRatio(*metrics.GRSignal);
        }
        };

    computeMetrics(uncompressedMetrics);
    computeMetrics(peakMetrics);
    computeMetrics(rmsMetrics);
}

// 1. Signal intensity and dynamic range metrics
//==============================================================================
float Metrics::getPeakValue(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    float peak = 0.0f;

    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = buffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n) {
            peak = std::max(peak, std::abs(channelData[n])); // Use absolute value here.
        }
    }
    return peak; // in linear gain
}

float Metrics::getAverageEnergy(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    float meanSquare = 0.0f;
    const float silenceThreshold = 0.0001f;

    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = buffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n) {
            if (std::abs(channelData[n]) >= silenceThreshold) {
                meanSquare += channelData[n] * channelData[n];
            }
        }
    }
    return meanSquare / static_cast<float>(numSamples * numChannels); // in linear gain
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
    const size_t n = shortTermLoudness.size();
    const float lowPercentile = shortTermLoudness[static_cast<size_t>(0.1 * n)];
    const float highPercentile = shortTermLoudness[static_cast<size_t>(0.95 * n)];

    // LRA is defined as the difference between the high and low percentiles
    return highPercentile - lowPercentile; // in db
}

// 2. Compression impact metrics
//==============================================================================
float Metrics::getDynamicRangeReductionCrest(float compressedCrestFactor)
{
    return uncompressedMetrics.crestFactor - compressedCrestFactor;  // in db
}

float Metrics::getDynamicRangeReductionLRA(float compressedLRA)
{
    return uncompressedMetrics.lra - compressedLRA; // in db
}

float Metrics::getTransientImpact(const juce::AudioBuffer<float>& compressedBuffer)
{
    constexpr float eps = 1.0e-12f;

    auto computeTransientStrengthPercentile = [](const juce::AudioBuffer<float>& buffer,
        float pct) -> float
        {
            const int numSamples = buffer.getNumSamples();
            const int numChannels = buffer.getNumChannels();

            if (numSamples < 2 || numChannels < 1)
                return 0.0f;

            std::vector<float> deltas;
            deltas.reserve((size_t)numChannels * (size_t)(numSamples - 1));

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* x = buffer.getReadPointer(ch);

                for (int n = 1; n < numSamples; ++n)
                    deltas.push_back(std::abs(x[n] - x[n - 1]));
            }

            if (deltas.empty())
                return 0.0f;

            pct = juce::jlimit(0.0f, 1.0f, pct);

            const size_t last = deltas.size() - 1;
            const size_t idx = (size_t)std::floor(pct * (float)last);

            // nth_element gives the element that would be at idx in sorted order, without fully sorting
            std::nth_element(deltas.begin(), deltas.begin() + (ptrdiff_t)idx, deltas.end());

            return deltas[idx]; // linear
        };

    const float uncompressedStrength =
        computeTransientStrengthPercentile(*uncompressedMetrics.signal, transientPercentile);

    const float compressedStrength =
        computeTransientStrengthPercentile(compressedBuffer, transientPercentile);

    if (uncompressedStrength <= eps)
        return 0.0f; // nothing to compare (silent or constant)

    // Signed impact:
    //  > 0  => transients got flatter (strength reduced)
    //  < 0  => transients got sharper (strength increased)
    float impact = 1.0f - (compressedStrength / uncompressedStrength);

    // Clamp to a sane range so one weird file doesn't explode your plots
    impact = juce::jlimit(-1.0f, 1.0f, impact);

    return impact;
}


float Metrics::getTransientEnergyPreservation(const juce::AudioBuffer<float>& compressedBuffer)
{
    // Split both signals into short-time windows
    std::vector<juce::AudioBuffer<float>> uncompressedWindows = getWindowsFromBuffer(*uncompressedMetrics.signal);
    std::vector<juce::AudioBuffer<float>> compressedWindows = getWindowsFromBuffer(compressedBuffer);

    if (uncompressedWindows.empty() || uncompressedWindows.size() != compressedWindows.size())
        return 0.0f;

    const size_t numWindows = uncompressedWindows.size();

    std::vector<float> uncompressedRMS;
    std::vector<float> compressedRMS;
    uncompressedRMS.reserve(numWindows);
    compressedRMS.reserve(numWindows);

    // Compute short-term RMS (linear) for each window
    for ( size_t k = 0; k < numWindows; ++k)
    {
        const float rmsUncompressed = getRMSValue(getAverageEnergy(uncompressedWindows[k]));
        const float rmsCompressed = getRMSValue(getAverageEnergy(compressedWindows[k]));

        uncompressedRMS.push_back(rmsUncompressed);
        compressedRMS.push_back(rmsCompressed);
    }

    // Mean short-term RMS of the uncompressed signal
    const float sumUncompressedRMS = std::accumulate(uncompressedRMS.begin(),
        uncompressedRMS.end(),
        0.0f);

    if (sumUncompressedRMS <= 0.0f)
        return 0.0f;

    const float meanUncompressedRMS = sumUncompressedRMS / static_cast<float>(numWindows);

    auto sorted = uncompressedRMS;
    std::sort(sorted.begin(), sorted.end());
    const float threshold = sorted[(size_t)(transientPercentile * (sorted.size() - 1))]; // top 10%

    // Accumulate energy in transient windows before and after compression
    float uncompressedEnergy = 0.0f;
    float compressedEnergy = 0.0f;

    for (size_t k = 0; k < numWindows; ++k)
    {
        const float rmsU = uncompressedRMS[k];

        if (rmsU > threshold)
        {
            const float rmsC = compressedRMS[k];

            // Energy ∝ RMS^2
            uncompressedEnergy += rmsU * rmsU;
            compressedEnergy += rmsC * rmsC;
        }
    }

    if (uncompressedEnergy <= 0.0f)
        return 0.0f;

    float ratio = compressedEnergy / uncompressedEnergy;

    // Clamp to [0, 1] – cannot preserve more than 100 % by definition here
    if (ratio > 1.0f)
        ratio = 1.0f;

    return ratio;
}

float Metrics::getWaveformDistortion(const juce::AudioBuffer<float>& compressedBuffer)
{
    const int numSamples = compressedBuffer.getNumSamples();
    const int numChannels = compressedBuffer.getNumChannels();

    const auto& originalBuffer = *uncompressedMetrics.signal;
    double originalEnergySum = 0.0;
    double errorEnergySum = 0.0;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* originalData = originalBuffer.getReadPointer(ch);
        const float* compressedData = compressedBuffer.getReadPointer(ch);

        for (int n = 0; n < numSamples; ++n)
        {
            const float x_u = originalData[n];
            const float x_c = compressedData[n];
            const float error = x_u - x_c;

            originalEnergySum += static_cast<double>(x_u) * static_cast<double>(x_u);
            errorEnergySum += static_cast<double>(error) * static_cast<double>(error);
        }
    }

    const int totalCount = numChannels * numSamples;
    if (totalCount <= 0)
        return 0.0f;

    const double originalMeanSquare = originalEnergySum / static_cast<double>(totalCount);
    const double errorMeanSquare = errorEnergySum / static_cast<double>(totalCount);

    const double originalRms = std::sqrt(originalMeanSquare);
    const double errorRms = std::sqrt(errorMeanSquare);

    const double eps = 1.0e-9;
    if (originalRms <= eps)
        return 0.0f;

    const double distortion = errorRms / originalRms;
    return static_cast<float>(distortion);
}

// 3. Gain reduction metrics
//==============================================================================
float Metrics::getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    const int numSamples = gainReductionBuffer.getNumSamples();
    const int numChannels = gainReductionBuffer.getNumChannels();

    float maxReduction = 0.0f;

    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = gainReductionBuffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n) {
            const float reduction = juce::Decibels::gainToDecibels(channelData[n]);
            if (reduction == -100.0f) { continue; } // ignore silence
            maxReduction = std::min(maxReduction, reduction); // Track max = min reduction
        }
    }
    return std::abs(maxReduction); // in db
}

float Metrics::getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    const int numSamples = gainReductionBuffer.getNumSamples();
    const int numChannels = gainReductionBuffer.getNumChannels();

    float totalGainReduction = 0.0f;

    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = gainReductionBuffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n) {
            const float reduction = juce::Decibels::gainToDecibels(channelData[n]);
            if (reduction == -100.0f) { continue; } // ignore silence
            totalGainReduction += std::fabs(reduction);   // Sum absolute reductions
        }
    }
    return totalGainReduction / (numSamples * numChannels); // in db
}

float Metrics::getStdDevGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer, float mean)
{
    const int numSamples = gainReductionBuffer.getNumSamples();
    const int numChannels = gainReductionBuffer.getNumChannels();

    float sum = 0.0f;

    // Step 2: Compute the sum of squared deviations from the mean
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = gainReductionBuffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n) {
            const float deviation = channelData[n] - mean;
            sum += deviation * deviation;
        }
    }
    return std::sqrt(sum / (numSamples * numChannels));
}

float Metrics::getEnergyGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    return std::abs(juce::Decibels::gainToDecibels(getAverageEnergy(gainReductionBuffer))); // in db
}

float Metrics::getRateOfChangeGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    const int numSamples = gainReductionBuffer.getNumSamples();
    const int numChannels = gainReductionBuffer.getNumChannels();

    float sumRateOfChange = 0.0f;

    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = gainReductionBuffer.getReadPointer(ch);
        for (int n = 1; n < numSamples; ++n) {
            sumRateOfChange += std::abs(channelData[n] - channelData[n - 1]);
        }
    }
    return (sumRateOfChange / ((numSamples - 1) * numChannels)) * sampleRate;
}

float Metrics::getCompressionActivityRatio(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    const int numSamples = gainReductionBuffer.getNumSamples();
    const int numChannels = gainReductionBuffer.getNumChannels();

    int activeSamples = 0;

    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = gainReductionBuffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n) {
            if (juce::Decibels::gainToDecibels(channelData[n]) < 0.0f) {
                ++activeSamples;
            }
        }
    }
    return static_cast<float>(activeSamples) / (numSamples * numChannels);
}


// Signal modifications
//==============================================================================

// Modifies input buffer for LUFS and LRA calculations by K-weighting
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

    for (size_t ch = 0; ch < static_cast<size_t>(audioBlock.getNumChannels()); ++ch) {
        auto singleChannelBlock = audioBlock.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);

        // Apply the filters
        lowShelfFilter.process(context);  // Apply low-shelf filter
        highShelfFilter.process(context); // Apply high-shelf filter
    }
}

// Create windows for short-term calculations
std::vector<juce::AudioBuffer<float>> Metrics::getWindowsFromBuffer(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const int windowSize = static_cast<int>(windowDuration * sampleRate);
    const int hopSize = static_cast<int>(hopDuration * sampleRate);

    std::vector<juce::AudioBuffer<float>> windowsBuffers;

    for (int startSample = 0; startSample + windowSize <= numSamples; startSample += hopSize) {
        juce::AudioBuffer<float> windowBuffer(numChannels, windowSize);
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* source = buffer.getReadPointer(ch, startSample);
            float* dest = windowBuffer.getWritePointer(ch);
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
        const float shortTermloudness = getIntegratedLoudness(getAverageEnergy(windowBuffer));
        shortTermLoudness.push_back(shortTermloudness);
    }
    return shortTermLoudness; // in db
}

// Signal verification
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

    const bool sameNumOfChannels =
        uncompressedMetrics.signal->getNumChannels() == peakMetrics.signal->getNumChannels() &&
        peakMetrics.signal->getNumChannels() == rmsMetrics.signal->getNumChannels() &&
        rmsMetrics.signal->getNumChannels() == peakMetrics.GRSignal->getNumChannels() &&
        peakMetrics.GRSignal->getNumChannels() == rmsMetrics.GRSignal->getNumChannels();

    const bool sameNumOfSamples =
        uncompressedMetrics.signal->getNumSamples() == peakMetrics.signal->getNumSamples() &&
        peakMetrics.signal->getNumSamples() == rmsMetrics.signal->getNumSamples() &&
        rmsMetrics.signal->getNumSamples() == peakMetrics.GRSignal->getNumSamples() &&
        peakMetrics.GRSignal->getNumSamples() == rmsMetrics.GRSignal->getNumSamples();

    return sameNumOfChannels && sameNumOfSamples;
}
