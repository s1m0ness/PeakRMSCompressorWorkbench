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
    sampleRate = fs / downSamplingFactor;
}

//==============================================================================
void Metrics::setUncompressedSignal(juce::AudioBuffer<float>* signal)
{
    uncompressedSignal = signal;
    uncompressedMetrics.signalName = "Uncompressed signal";
    uncompressedMetrics.trackGR = false;
}

void Metrics::setPeakCompressedSignal(juce::AudioBuffer<float>* signal)
{
    peakCompressedSignal = signal;
    peakMetrics.signalName = "Peak compressed signal";
    peakMetrics.trackGR = true;
}

void Metrics::setRMSCompressedSignal(juce::AudioBuffer<float>* signal)
{
    rmsCompressedSignal = signal;
    rmsMetrics.signalName = "RMS compressed signal";
    rmsMetrics.trackGR = true;
}

void Metrics::setPeakGainReductionSignal(juce::AudioBuffer<float>* signal)
{
    peakGainReductionSignal = signal;
}

void Metrics::setRMSGainReductionSignal(juce::AudioBuffer<float>* signal)
{
    rmsGainReductionSignal = signal;
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
void Metrics::extractMetrics(float peakRatio, float rmsRatio)
{
    // Check for empty signals 
    if (!validateSignal(*uncompressedSignal) || 
        !validateSignal(*peakCompressedSignal) || 
        !validateSignal(*peakGainReductionSignal) ||
        !validateSignal(*rmsCompressedSignal) ||
        !validateSignal(*rmsGainReductionSignal)) {
        return;
    }

    // Downsampling to better highlight the differences 
    // in peak and rms compression behaviour
    downsampleBuffer(uncompressedSignal);
    downsampleBuffer(peakCompressedSignal);
    downsampleBuffer(rmsCompressedSignal);
    downsampleBuffer(peakGainReductionSignal);
    downsampleBuffer(rmsGainReductionSignal);

    // Uncompressed Signal Metrics
    uncompressedMetrics.meanSquare = getMeanSquareEnergy(*uncompressedSignal);
    uncompressedMetrics.peak = getPeakValue(*uncompressedSignal);
    uncompressedMetrics.rms = getRMSValue(uncompressedMetrics.meanSquare);
    uncompressedMetrics.crestFactor = getCrestFactor(uncompressedMetrics.peak, uncompressedMetrics.rms);
    uncompressedMetrics.lufs = getLUFS(*uncompressedSignal);

    // Peak Compressed Signal Metrics
    peakMetrics.meanSquare = getMeanSquareEnergy(*peakCompressedSignal);
    peakMetrics.peak = getPeakValue(*peakCompressedSignal);
    peakMetrics.rms = getRMSValue(peakMetrics.meanSquare);
    peakMetrics.crestFactor = getCrestFactor(peakMetrics.peak, peakMetrics.rms);
    peakMetrics.lufs = getLUFS(*peakCompressedSignal);
    peakMetrics.dynamicRangeReduction = getDynamicRangeReduction(false);
    peakMetrics.avgGR = getAverageGainReduction(*peakGainReductionSignal);
    peakMetrics.maxGR = getMaxGainReduction(*peakGainReductionSignal);
    
    // RMS Compressed Signal Metrics
    rmsMetrics.meanSquare = getMeanSquareEnergy(*rmsCompressedSignal);
    rmsMetrics.peak = getPeakValue(*rmsCompressedSignal);
    rmsMetrics.rms = getRMSValue(rmsMetrics.meanSquare);
    rmsMetrics.crestFactor = getCrestFactor(rmsMetrics.peak, rmsMetrics.rms);
    rmsMetrics.lufs = getLUFS(*rmsCompressedSignal);
    rmsMetrics.dynamicRangeReduction = getDynamicRangeReduction(true);
    rmsMetrics.avgGR = getAverageGainReduction(*rmsGainReductionSignal);
    rmsMetrics.maxGR = getMaxGainReduction(*rmsGainReductionSignal);

    //peakMetrics.rmse = getRMSE(uncompressedSignal, peakCompressedSignal);
    //peakMetrics.correlation = getCorrelation(uncompressedSignal, peakCompressedSignal);
    //peakMetrics.LRA = getLRA(*peakCompressedSignal, sampleRate, 0.4f, 0.2f);
    //peakMetrics.stdDevGR = getGainReductionStdDev(*peakGainReductionSignal);
    //peakMetrics.compressionActivity = getCompressionActivityRatio(peakGainReductionSignal);
    //peakMetrics.crestGR = getGainReductionCrestFactor(peakGainReductionSignal);
    //peakMetrics.dynamicRangeReduction = getDynamicRangeReduction(uncompressedSignal, peakCompressedSignal);
    //peakMetrics.transientImpact = getTransientImpact(uncompressedSignal, peakCompressedSignal);
    //peakMetrics.rateOfChangeGR = getGainReductionRateOfChange(peakGainReductionSignal, sampleRate);
    //peakMetrics.gainReductionEnergy = getGainReductionEnergy(peakGainReductionSignal);
    //peakMetrics.ratioEfficiency = getRatioEfficiency(uncompressedSignal, peakGainReductionSignal, peakRatio);
    //peakMetrics.attackReleaeTime = getAttackReleaseTime(peakGainReductionSignal, sampleRate);
    //peakMetrics.temporalSmoothness = getTemporalSmoothness(peakGainReductionSignal);
    //peakMetrics.transientEnergyPreservation = getTransientEnergyPreservation(uncompressedSignal, peakCompressedSignal);

    
    //rmsMetrics.rmse = getRMSE(uncompressedSignal, rmsCompressedSignal);
    //rmsMetrics.correlation = getCorrelation(uncompressedSignal, rmsCompressedSignal);
    //rmsMetrics.LRA = getLRA(*rmsCompressedSignal, sampleRate, 0.4f, 0.2f);
    //rmsMetrics.stdDevGR = getGainReductionStdDev(*rmsGainReductionSignal);
    //rmsMetrics.compressionActivity = getCompressionActivityRatio(rmsGainReductionSignal);
    //rmsMetrics.crestGR = getGainReductionCrestFactor(rmsGainReductionSignal);
    //rmsMetrics.dynamicRangeReduction = getDynamicRangeReduction(uncompressedSignal, rmsCompressedSignal);
    //rmsMetrics.transientImpact = getTransientImpact(uncompressedSignal, rmsCompressedSignal);
    //rmsMetrics.rateOfChangeGR = getGainReductionRateOfChange(rmsGainReductionSignal, sampleRate);
    //rmsMetrics.gainReductionEnergy = getGainReductionEnergy(rmsGainReductionSignal);
    //rmsMetrics.ratioEfficiency = getRatioEfficiency(uncompressedSignal, rmsGainReductionSignal, rmsRatio);
    //rmsMetrics.attackReleaeTime = getAttackReleaseTime(rmsGainReductionSignal, sampleRate);
    //rmsMetrics.temporalSmoothness = getTemporalSmoothness(rmsGainReductionSignal);
    //rmsMetrics.transientEnergyPreservation = getTransientEnergyPreservation(uncompressedSignal, rmsCompressedSignal);
}

//==============================================================================
//float Metrics::getRMSE(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const
//{
//    if (!validateSignals(signalA, signalB))
//        return 0.0f;
//
//    const int numSamples = signalA->getNumSamples();
//    const int numChannels = std::min(signalA->getNumChannels(), signalB->getNumChannels());
//
//    float mse = 0.0f;
//
//    for (int channel = 0; channel < numChannels; ++channel)
//    {
//        const float* samplesA = signalA->getReadPointer(channel);
//        const float* samplesB = signalB->getReadPointer(channel);
//
//        for (int i = 0; i < numSamples; ++i)
//        {
//            float diff = samplesA[i] - samplesB[i];
//            mse += diff * diff;
//        }
//    }
//
//    mse /= numSamples;
//    return std::sqrt(mse);
//}
//
//float Metrics::getCorrelation(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const
//{
//    if (!validateSignals(signalA, signalB))
//        return 0.0f;
//
//    const int numSamples = signalA->getNumSamples();
//    const int numChannels = std::min(signalA->getNumChannels(), signalB->getNumChannels());
//
//    float crossProduct = 0.0f;
//    float sumASquared = 0.0f;
//    float sumBSquared = 0.0f;
//
//    for (int channel = 0; channel < numChannels; ++channel)
//    {
//        const float* samplesA = signalA->getReadPointer(channel);
//        const float* samplesB = signalB->getReadPointer(channel);
//
//        // Use std::inner_product for dot products
//        crossProduct += std::inner_product(samplesA, samplesA + numSamples, samplesB, 0.0f);
//        sumASquared += std::inner_product(samplesA, samplesA + numSamples, samplesA, 0.0f);
//        sumBSquared += std::inner_product(samplesB, samplesB + numSamples, samplesB, 0.0f);
//    }
//
//    float denominator = std::sqrt(sumASquared * sumBSquared);
//    return (denominator > 0) ? (crossProduct / denominator) : 0.0f;
//}
//
//float Metrics::getCrestFactor(const juce::AudioBuffer<float>& buffer) const
//{
//    float peak = 0.0f;
//    float rms = 0.0f;
//
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        const float* samples = buffer.getReadPointer(channel);
//        for (int i = 0; i < buffer.getNumSamples(); ++i)
//        {
//            peak = std::max(peak, std::abs(samples[i]));
//            rms += samples[i] * samples[i];
//        }
//    }
//
//    rms = std::sqrt(rms / (buffer.getNumChannels() * buffer.getNumSamples()));
//
//    return peak / rms;
//}
//
//float Metrics::getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
//{
//    // Ensure the buffer has valid samples
//    if (gainReductionBuffer.getNumSamples() == 0 || gainReductionBuffer.getNumChannels() == 0)
//        return 0.0f;
//
//    float maxReduction = 0;
//
//    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
//    {
//        const float* channelData = gainReductionBuffer.getReadPointer(channel);
//        for (int sample = 0; sample < gainReductionBuffer.getNumSamples(); ++sample)
//        {
//            float reduction = juce::Decibels::gainToDecibels(channelData[sample]);
//            if (reduction == -100) { continue; }
//            maxReduction = std::min(maxReduction, reduction); // Track maximum reduction
//        }
//    }
//
//    return maxReduction;
//}
//
//float Metrics::getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
//{
//    // Ensure the buffer has valid samples
//    if (gainReductionBuffer.getNumSamples() == 0 || gainReductionBuffer.getNumChannels() == 0)
//        return 0.0f;
//
//    float totalGainReduction = 0.0f;
//    int totalSamples = 0;
//
//    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
//    {
//        const float* channelData = gainReductionBuffer.getReadPointer(channel);
//        for (int sample = 0; sample < gainReductionBuffer.getNumSamples(); ++sample)
//        {
//            float reduction = juce::Decibels::gainToDecibels(channelData[sample]); // Convert gain to reduction
//            if (reduction == -100) { continue; }
//            totalGainReduction += std::fabs(reduction);   // Sum absolute reductions
//        }
//        totalSamples += gainReductionBuffer.getNumSamples();
//    }
//
//    return (totalSamples > 0) ? ((totalGainReduction / totalSamples)) : 0.0f;
//}
//
//float Metrics::getLRA(const juce::AudioBuffer<float>& buffer, double sampleRate, float windowDuration, float hopDuration) const
//{
//    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
//        return 0.0f;
//
//    // Parameters for sliding window
//    const int windowSize = static_cast<int>(windowDuration * sampleRate);
//    const int hopSize = static_cast<int>(hopDuration * sampleRate);
//
//    // Prepare K-Weighted Buffer
//    juce::AudioBuffer<float> kWeightedBuffer;
//    kWeightedBuffer.makeCopyOf(buffer);
//
//    // init filters
//    juce::dsp::IIR::Filter<float> lowShelfFilter, highShelfFilter;
//
//    // Apply K-weighting filters
//    lowShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
//        sampleRate, 1681.974450955533, 0.7071752369554196, 1.53512485958697);
//    highShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
//        sampleRate, 424.318677406412, 0.7071752369554196, 1.0);
//
//    // Convert the buffer into a JUCE AudioBlock
//    juce::dsp::AudioBlock<float> audioBlock(kWeightedBuffer);
//
//    // Apply the filters channel by channel
//    for (size_t channel = 0; channel < audioBlock.getNumChannels(); ++channel)
//    {
//        juce::dsp::ProcessContextReplacing<float> context(audioBlock.getSingleChannelBlock(channel));
//        lowShelfFilter.process(context); // Apply the low-shelf filter
//        highShelfFilter.process(context); // Apply the high-shelf filter
//    }
//
//    // Step 1: Calculate short-term loudness in sliding windows
//    std::vector<float> shortTermLoudness;
//
//    for (int startSample = 0; startSample + windowSize <= kWeightedBuffer.getNumSamples(); startSample += hopSize)
//    {
//        float sumSquares = 0.0f;
//
//        for (int channel = 0; channel < kWeightedBuffer.getNumChannels(); ++channel)
//        {
//            const float* samples = kWeightedBuffer.getReadPointer(channel);
//            for (int i = startSample; i < startSample + windowSize; ++i)
//            {
//                sumSquares += samples[i] * samples[i];
//            }
//        }
//
//        int numSamplesInWindow = windowSize * kWeightedBuffer.getNumChannels();
//        float rms = std::sqrt(sumSquares / numSamplesInWindow);
//
//        // Convert RMS to LUFS for this window
//        if (rms > 0.0f)
//        {
//            float lufs = 10.0f * std::log10(rms);
//            shortTermLoudness.push_back(lufs);
//        }
//    }
//
//    // Step 2: Calculate LRA from short-term loudness
//    if (shortTermLoudness.empty())
//        return 0.0f;
//
//    // Sort loudness values to calculate percentiles
//    std::sort(shortTermLoudness.begin(), shortTermLoudness.end());
//
//    // Calculate 10th and 95th percentiles
//    size_t n = shortTermLoudness.size();
//    float lowPercentile = shortTermLoudness[static_cast<size_t>(0.1 * n)];
//    float highPercentile = shortTermLoudness[static_cast<size_t>(0.95 * n)];
//
//    return highPercentile - lowPercentile;
//}
//
//float Metrics::getGainReductionStdDev(const juce::AudioBuffer<float>& gainReductionBuffer)
//{
//    float sum = 0.0f;
//    float mean = 0.0f;
//    int totalSamples = gainReductionBuffer.getNumSamples();
//
//    // Step 1: Compute the mean gain reduction
//    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
//    {
//        const float* channelData = gainReductionBuffer.getReadPointer(channel);
//        for (int sample = 0; sample < totalSamples; ++sample)
//        {
//            mean += channelData[sample];
//        }
//    }
//    mean /= (gainReductionBuffer.getNumChannels() * totalSamples);
//
//    // Step 2: Compute the sum of squared deviations from the mean
//    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
//    {
//        const float* channelData = gainReductionBuffer.getReadPointer(channel);
//        for (int sample = 0; sample < totalSamples; ++sample)
//        {
//            float deviation = channelData[sample] - mean;
//            sum += deviation * deviation;
//        }
//    }
//
//    // Step 3: Return the standard deviation
//    return std::sqrt(sum / (gainReductionBuffer.getNumChannels() * totalSamples));
//}
//
//float Metrics::getCompressionActivityRatio(const juce::AudioBuffer<float>* gainReductionSignal)
//{
//    // Check for signal validity
//    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
//    {
//        DBG("Invalid or empty gain reduction signal.");
//        return 0.0f;
//    }
//
//    /*DBG("Number of samples because i am a dumb motherfucker: " << gainReductionSignal->getNumSamples());*/
//
//    const auto* data = gainReductionSignal->getReadPointer(0);
//    int activeSamples = 0;
//
//    DBG("Inspecting Gain Reduction Signal:");
//    for (int i = 0; i < gainReductionSignal->getNumSamples(); ++i)
//    {
//        // Check activity using absolute value
//        if (juce::Decibels::gainToDecibels(data[i]) < 0)
//            ++activeSamples;
//    }
//
//    if (activeSamples == 0)
//    {
//        DBG("No active samples found in gain reduction signal.");
//    }
//
//    return static_cast<float>(activeSamples) / static_cast<float>(gainReductionSignal->getNumSamples());
//}
//
//float Metrics::getGainReductionCrestFactor(const juce::AudioBuffer<float>* gainReductionSignal)
//{
//    // Check for signal validity
//    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
//        return 0.0f;
//
//    const auto* data = gainReductionSignal->getReadPointer(0);
//    int numSamples = gainReductionSignal->getNumSamples();
//
//    // Find the peak and RMS values
//    float peak = 0.0f;
//    float rms = 0.0f;
//    for (int i = 0; i < numSamples; ++i)
//    {
//        float absValue = std::abs(data[i]);
//        peak = std::max(peak, absValue);
//        rms += absValue * absValue;
//    }
//
//    rms = std::sqrt(rms / numSamples);
//
//    // Avoid division by zero
//    if (rms == 0.0f)
//        return 0.0f;
//
//    return peak / rms;
//}
// 
//float Metrics::getTransientImpact(const juce::AudioBuffer<float>* uncompressedSignal,
//    const juce::AudioBuffer<float>* compressedSignal) {
//    if (!uncompressedSignal || !compressedSignal ||
//        uncompressedSignal->getNumSamples() == 0 || compressedSignal->getNumSamples() == 0)
//        return 0.0f;
//
//    auto computeTransientAmplitudeDb = [](const juce::AudioBuffer<float>* signal) {
//        const float* data = signal->getReadPointer(0);
//        int numSamples = signal->getNumSamples();
//        float maxDeltaDb = -std::numeric_limits<float>::infinity();
//
//        for (int i = 1; i < numSamples; ++i) {
//            float deltaDb = juce::Decibels::gainToDecibels(std::abs(data[i] - data[i - 1]), -100.0f);
//            maxDeltaDb = std::max(maxDeltaDb, deltaDb);
//        }
//        return maxDeltaDb;
//        };
//
//    float uncompressedTransients = computeTransientAmplitudeDb(uncompressedSignal);
//    float compressedTransients = computeTransientAmplitudeDb(compressedSignal);
//
//    // Normalize by uncompressed transient level
//    return uncompressedTransients > 0.0f ? (uncompressedTransients - compressedTransients) / uncompressedTransients : 0.0f;
//}
//
//float Metrics::getGainReductionRateOfChange(const juce::AudioBuffer<float>* gainReductionSignal, float sampleRate)
//{
//    // Check for signal validity
//    if (!gainReductionSignal || gainReductionSignal->getNumSamples() < 2)
//        return 0.0f;
//
//    const auto* data = gainReductionSignal->getReadPointer(0);
//    int numSamples = gainReductionSignal->getNumSamples();
//
//    // Compute rate of change
//    float sumRateOfChange = 0.0f;
//    for (int i = 1; i < numSamples; ++i)
//    {
//        sumRateOfChange += std::abs(data[i] - data[i - 1]);
//    }
//
//    // Normalize to time
//    return (sumRateOfChange / (numSamples - 1)) * sampleRate;
//}
//
//float Metrics::getGainReductionEnergy(const juce::AudioBuffer<float>* gainReductionSignal) {
//    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
//        return 0.0f;
//
//    const auto* data = gainReductionSignal->getReadPointer(0);
//    float energy = 0.0f;
//
//    for (int i = 0; i < gainReductionSignal->getNumSamples(); ++i) {
//        energy += data[i] * data[i];
//    }
//    
//    energy /= gainReductionSignal->getNumSamples();
//
//    return juce::Decibels::gainToDecibels(energy, -100.0f); // Convert to dB for interpretability
//}
//
//float Metrics::getRatioEfficiency(const juce::AudioBuffer<float>* inputSignal, const juce::AudioBuffer<float>* gainReductionSignal, float ratioInDb) {
//    if (!inputSignal || !gainReductionSignal ||
//        inputSignal->getNumSamples() != gainReductionSignal->getNumSamples() || ratioInDb <= 0.0f)
//        return 0.0f;
//
//    // Convert the ratio from dB to linear
//    float linearRatio = juce::Decibels::decibelsToGain(ratioInDb);
//
//    const auto* inputData = inputSignal->getReadPointer(0);
//    const auto* grData = gainReductionSignal->getReadPointer(0);
//    float efficiencySum = 0.0f;
//    float weightSum = 0.0f;
//
//    for (int i = 0; i < inputSignal->getNumSamples(); ++i) {
//        float inputAmplitude = std::abs(inputData[i]);
//        if (inputAmplitude <= 0.0f) continue; // Skip zero or negative inputs
//
//        // Compute theoretical reduction
//        float theoreticalReduction = inputAmplitude - (inputAmplitude / linearRatio);
//        if (theoreticalReduction <= 0.0f) continue; // Skip invalid reductions
//
//        // Calculate efficiency as the ratio of actual to theoretical reduction
//        float efficiency = std::abs(grData[i] / theoreticalReduction);
//
//        // Clamp outlier efficiency values to prevent distortion
//        if (efficiency > 10.0f) efficiency = 10.0f;
//
//        // Weight efficiency by input amplitude
//        efficiencySum += efficiency * inputAmplitude;
//        weightSum += inputAmplitude;
//    }
//
//    return weightSum > 0.0f ? (efficiencySum / weightSum) : 0.0f;
//}
//
//float Metrics::getAttackReleaseTime(const juce::AudioBuffer<float>* gainReductionSignal, float sampleRate) {
//    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
//        return 0.0f;
//
//    const auto* data = gainReductionSignal->getReadPointer(0);
//    int numSamples = gainReductionSignal->getNumSamples();
//
//    // Track maximum reduction and its sample index
//    float maxReductionDb = 0;
//    int maxReductionSample = 0;
//
//    // Find the maximum reduction in dB
//    for (int i = 0; i < numSamples; ++i) {
//        float reductionDb = juce::Decibels::gainToDecibels(data[i], -100.0f);
//        if (reductionDb < maxReductionDb) { // Find the most negative value (maximum reduction)
//            maxReductionDb = reductionDb;
//            maxReductionSample = i;
//        }
//    }
//
//    // Stabilization threshold: 10% of max reduction
//    float stabilizationThresholdDb = maxReductionDb * 0.1f;
//    int stabilizationSample = maxReductionSample;
//
//    // Iterate to find when gain reduction stabilizes
//    for (int i = maxReductionSample; i < numSamples; ++i) {
//        float reductionDb = juce::Decibels::gainToDecibels(data[i], -100.0f);
//        if (reductionDb >= stabilizationThresholdDb) { // Stabilization reached
//            stabilizationSample = i;
//            break;
//        }
//    }
//
//    // Calculate stabilization time
//    return static_cast<float>(stabilizationSample - maxReductionSample) / sampleRate;
//}
//
//float Metrics::getTemporalSmoothness(const juce::AudioBuffer<float>* gainReductionSignal) {
//    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
//        return 0.0f;
//
//    const auto* data = gainReductionSignal->getReadPointer(0);
//    int numSamples = gainReductionSignal->getNumSamples();
//
//    // Compute mean of the gain reduction signal
//    float mean = 0.0f;
//    for (int i = 0; i < numSamples; ++i) {
//        mean += data[i];
//    }
//    mean /= numSamples;
//
//    // Compute variance
//    float variance = 0.0f;
//    for (int i = 0; i < numSamples; ++i) {
//        float diff = data[i] - mean;
//        variance += diff * diff;
//    }
//    variance /= numSamples;
//
//    return variance; // Variance as a measure of temporal smoothness
//}
//
//float Metrics::getTransientEnergyPreservation(const juce::AudioBuffer<float>* uncompressedSignal,
//    const juce::AudioBuffer<float>* compressedSignal) {
//    if (!uncompressedSignal || !compressedSignal ||
//        uncompressedSignal->getNumSamples() != compressedSignal->getNumSamples()) {
//        return 0.0f; // Return zero for invalid or mismatched signals
//    }
//
//    const int numSamples = uncompressedSignal->getNumSamples();
//    const int numChannels = std::min(uncompressedSignal->getNumChannels(), compressedSignal->getNumChannels());
//    const int windowSize = 88; // 20 ms at 4.41 kHz
//    const float transientThresholdMultiplier = 3.0f; // Multiplier for transient threshold
//
//    // Calculate mean RMS energy of the uncompressed signal
//    float meanRMS = 0.0f;
//    for (int channel = 0; channel < numChannels; ++channel) {
//        const float* data = uncompressedSignal->getReadPointer(channel);
//        for (int i = 0; i < numSamples; ++i) {
//            meanRMS += data[i] * data[i];
//        }
//    }
//    meanRMS = std::sqrt(meanRMS / (numSamples * numChannels));
//    const float threshold = transientThresholdMultiplier * meanRMS;
//
//    float uncompressedEnergy = 0.0f;
//    float compressedEnergy = 0.0f;
//
//    // Process each channel and accumulate transient energy
//    for (int channel = 0; channel < numChannels; ++channel) {
//        const float* uncompressedData = uncompressedSignal->getReadPointer(channel);
//        const float* compressedData = compressedSignal->getReadPointer(channel);
//
//        for (int i = 0; i < numSamples - windowSize; i += windowSize) {
//            // Compute short-term RMS for the uncompressed signal
//            float rmsUncompressed = 0.0f;
//            for (int j = 0; j < windowSize; ++j) {
//                rmsUncompressed += uncompressedData[i + j] * uncompressedData[i + j];
//            }
//            rmsUncompressed = std::sqrt(rmsUncompressed / windowSize);
//
//            // If the uncompressed RMS exceeds the transient threshold, calculate transient energy
//            if (rmsUncompressed > threshold) {
//                // Compute short-term RMS for the compressed signal
//                float rmsCompressed = 0.0f;
//                for (int j = 0; j < windowSize; ++j) {
//                    rmsCompressed += compressedData[i + j] * compressedData[i + j];
//                }
//                rmsCompressed = std::sqrt(rmsCompressed / windowSize);
//
//                // Accumulate transient energy
//                uncompressedEnergy += rmsUncompressed * rmsUncompressed; // Energy is RMS^2
//                compressedEnergy += rmsCompressed * rmsCompressed;       // Energy is RMS^2
//            }
//        }
//    }
//
//    // Compute and return preservation ratio
//    return (uncompressedEnergy > 0.0f) ? std::min(compressedEnergy / uncompressedEnergy, 1.0f) : 0.0f;
//}

// LINEAR GAIN
//==============================================================================
float Metrics::getMeanSquareEnergy(const juce::AudioBuffer<float>& signal)
{
    float meanSquare = 0.0f;
    int totalSamples = signal.getNumSamples();
    int totalChannels = signal.getNumChannels();

    const float silenceThreshold = 0.0001f;
    int totalSampleCount = 0;

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = signal.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            if (std::abs(channelData[sample]) >= silenceThreshold) {
                meanSquare += channelData[sample] * channelData[sample];
                ++totalSampleCount;
            }
        }
    }
    if (totalSampleCount == 0)
        return 0.0f;

    return meanSquare / static_cast<float>(totalSampleCount);
}

float Metrics::getPeakValue(const juce::AudioBuffer<float>& signal) {
    float peak = 0.0f;
    int totalSamples = signal.getNumSamples();
    int totalChannels = signal.getNumChannels();

    for (int channel = 0; channel < totalChannels; ++channel) {
        const float* channelData = signal.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample) {
            peak = std::max(peak, std::abs(channelData[sample])); // Use absolute value here.
        }
    }
    return peak;
}

float Metrics::getRMSValue(float meanSquare) 
{
    return std::sqrtf(meanSquare);
}

// LOG (dB)
//==============================================================================
float Metrics::getCrestFactor(float peakValue, float rmsValue) 
{
    return juce::Decibels::gainToDecibels(peakValue / rmsValue);
}

float Metrics::getLUFS(const juce::AudioBuffer<float>& buffer)
{
    // Copy the buffer for K-weighting
    juce::AudioBuffer<float> kWeightedBuffer;
    kWeightedBuffer.makeCopyOf(buffer);

    // Apply K-weighting filters
    applyKWeighting(kWeightedBuffer);

    return getIntegratedLoudness(getMeanSquareEnergy(kWeightedBuffer));
}

float Metrics::getDynamicRangeReduction(bool isRMS) {
   
    float uncompressedRange = uncompressedMetrics.crestFactor;
    
    float compressedRange = 0.0f;
    if (!isRMS) {
        compressedRange = peakMetrics.crestFactor;
    } else {
        compressedRange = rmsMetrics.crestFactor;
    }

    return uncompressedRange - compressedRange;
    
}

float Metrics::getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    float maxReduction = 0;

    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < gainReductionBuffer.getNumSamples(); ++sample) {
            float reduction = juce::Decibels::gainToDecibels(channelData[sample]);
            if (reduction == -100) { continue; } // ignore silence
            maxReduction = std::min(maxReduction, reduction); // Track max = min reduction
        }
    }
    return std::abs(maxReduction);
}

float Metrics::getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    float totalGainReduction = 0.0f;
    int totalSamples = 0;

    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel) {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < gainReductionBuffer.getNumSamples(); ++sample) {
            float reduction = juce::Decibels::gainToDecibels(channelData[sample]); // Convert gain to reduction
            if (reduction == -100) { continue; } // ignore silence
            totalGainReduction += std::fabs(reduction);   // Sum absolute reductions
        }
        totalSamples += gainReductionBuffer.getNumSamples();
    }
    return (totalSamples > 0) ? ((totalGainReduction / totalSamples)) : 0.0f;
}

// Signal handling
//==============================================================================
bool Metrics::validateSignal(const juce::AudioBuffer<float>& signal) const
{
    if (signal.getNumSamples() == 0 || signal.getNumChannels() == 0) {
        return false;
    }
    return true;
}

void Metrics::downsampleBuffer(juce::AudioBuffer<float>* buffer) {

    int numSamples = buffer->getNumSamples();
    int numChannels = buffer->getNumChannels();
    int downsampledSamples = numSamples / downSamplingFactor;

    juce::AudioBuffer<float> tempBuffer(numChannels, downsampledSamples);

    for (int channel = 0; channel < numChannels; ++channel) {
        const float* input = buffer->getReadPointer(channel);
        float* output = tempBuffer.getWritePointer(channel);
        for (int i = 0; i < downsampledSamples; ++i) {
            output[i] = input[i * downSamplingFactor];
        }
    }
    *buffer = std::move(tempBuffer);
}

// Additional computation
//==============================================================================
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

float Metrics::getIntegratedLoudness(float meanSquare)
{
    // Convert mean square to LUFS using JUCE decibel utilities
    return juce::Decibels::gainToDecibels(meanSquare) - 0.691f; // K-weighting correction
}
