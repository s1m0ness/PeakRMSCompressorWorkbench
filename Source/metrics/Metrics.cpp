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
void Metrics::setUncompressedSignal(const juce::AudioBuffer<float>* signal)
{
    uncompressedSignal = signal;
}

void Metrics::setPeakCompressedSignal(const juce::AudioBuffer<float>* signal)
{
    peakCompressedSignal = signal;
}

void Metrics::setPeakGainReductionSignal(const juce::AudioBuffer<float>* signal)
{
    peakGainReductionSignal = signal;
}

void Metrics::setRMSCompressedSignal(const juce::AudioBuffer<float>* signal)
{
    rmsCompressedSignal = signal;
}

void Metrics::setRMSGainReductionSignal(const juce::AudioBuffer<float>* signal)
{
    rmsGainReductionSignal = signal;
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
void Metrics::calculateMetrics(float peakRatio, float rmsRatio)
{
    if (uncompressedSignal && peakCompressedSignal)
    {
        // Peak Metrics
        peakMetrics.rmse = getRMSE(uncompressedSignal, peakCompressedSignal);
        peakMetrics.correlation = getCorrelation(uncompressedSignal, peakCompressedSignal);
        peakMetrics.crest = getCrestFactor(*peakCompressedSignal);
        peakMetrics.avgGR = getAverageGainReduction(*peakGainReductionSignal);
        peakMetrics.maxGR = getMaxGainReduction(*peakGainReductionSignal);
        peakMetrics.LRA = getLRA(*peakCompressedSignal, sampleRate, 0.4f, 0.2f);
        peakMetrics.stdDevGR = getGainReductionStdDev(*peakGainReductionSignal);
        peakMetrics.compressionActivity = getCompressionActivityRatio(peakGainReductionSignal);
        peakMetrics.crestGR = getGainReductionCrestFactor(peakGainReductionSignal);
        peakMetrics.dynamicRangeReduction = getDynamicRangeReduction(uncompressedSignal, peakCompressedSignal);
        peakMetrics.transientImpact = getTransientImpact(uncompressedSignal, peakCompressedSignal);
        peakMetrics.rateOfChangeGR = getGainReductionRateOfChange(peakGainReductionSignal, sampleRate);

        peakMetrics.gainReductionEnergy = getGainReductionEnergy(peakGainReductionSignal);
        peakMetrics.ratioEfficiency = getRatioEfficiency(uncompressedSignal, peakGainReductionSignal, peakRatio);

        //peakMetrics.harmonicDistortion = getHarmonicDistortion(uncompressedSignal, peakCompressedSignal);
    }

    if (uncompressedSignal && rmsCompressedSignal)
    {
        // RMS Metrics
        rmsMetrics.rmse = getRMSE(uncompressedSignal, rmsCompressedSignal);
        rmsMetrics.correlation = getCorrelation(uncompressedSignal, rmsCompressedSignal);
        rmsMetrics.crest = getCrestFactor(*rmsCompressedSignal);
        rmsMetrics.avgGR = getAverageGainReduction(*rmsGainReductionSignal);
        rmsMetrics.maxGR = getMaxGainReduction(*rmsGainReductionSignal);
        rmsMetrics.maxGR = getMaxGainReduction(*rmsGainReductionSignal);
        rmsMetrics.LRA = getLRA(*rmsCompressedSignal, sampleRate, 0.4f, 0.2f);
        rmsMetrics.stdDevGR = getGainReductionStdDev(*rmsGainReductionSignal);
        rmsMetrics.compressionActivity = getCompressionActivityRatio(rmsGainReductionSignal);
        rmsMetrics.crestGR = getGainReductionCrestFactor(rmsGainReductionSignal);
        rmsMetrics.dynamicRangeReduction = getDynamicRangeReduction(uncompressedSignal, rmsCompressedSignal);
        rmsMetrics.transientImpact = getTransientImpact(uncompressedSignal, rmsCompressedSignal);
        rmsMetrics.rateOfChangeGR = getGainReductionRateOfChange(rmsGainReductionSignal, sampleRate);

        rmsMetrics.gainReductionEnergy = getGainReductionEnergy(rmsGainReductionSignal);
        rmsMetrics.ratioEfficiency = getRatioEfficiency(uncompressedSignal, rmsGainReductionSignal, rmsRatio);

        //rmsMetrics.harmonicDistortion = getHarmonicDistortion(uncompressedSignal, rmsCompressedSignal);
    }
}


//==============================================================================
float Metrics::getRMSE(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const
{
    if (!validateSignals(signalA, signalB))
        return 0.0f;

    const int numSamples = signalA->getNumSamples();
    const int numChannels = std::min(signalA->getNumChannels(), signalB->getNumChannels());

    float mse = 0.0f;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* samplesA = signalA->getReadPointer(channel);
        const float* samplesB = signalB->getReadPointer(channel);

        for (int i = 0; i < numSamples; ++i)
        {
            float diff = samplesA[i] - samplesB[i];
            mse += diff * diff;
        }
    }

    mse /= numSamples;
    return std::sqrt(mse);
}

float Metrics::getCorrelation(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const
{
    if (!validateSignals(signalA, signalB))
        return 0.0f;

    const int numSamples = signalA->getNumSamples();
    const int numChannels = std::min(signalA->getNumChannels(), signalB->getNumChannels());

    float crossProduct = 0.0f;
    float sumASquared = 0.0f;
    float sumBSquared = 0.0f;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* samplesA = signalA->getReadPointer(channel);
        const float* samplesB = signalB->getReadPointer(channel);

        // Use std::inner_product for dot products
        crossProduct += std::inner_product(samplesA, samplesA + numSamples, samplesB, 0.0f);
        sumASquared += std::inner_product(samplesA, samplesA + numSamples, samplesA, 0.0f);
        sumBSquared += std::inner_product(samplesB, samplesB + numSamples, samplesB, 0.0f);
    }

    float denominator = std::sqrt(sumASquared * sumBSquared);
    return (denominator > 0) ? (crossProduct / denominator) : 0.0f;
}

float Metrics::getCrestFactor(const juce::AudioBuffer<float>& buffer) const
{
    float peak = 0.0f;
    float rms = 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float* samples = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            peak = std::max(peak, std::abs(samples[i]));
            rms += samples[i] * samples[i];
        }
    }

    rms = std::sqrt(rms / (buffer.getNumChannels() * buffer.getNumSamples()));

    return peak / rms;
}

float Metrics::getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    // Ensure the buffer has valid samples
    if (gainReductionBuffer.getNumSamples() == 0 || gainReductionBuffer.getNumChannels() == 0)
        return 0.0f;

    float maxReduction = 0;

    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
    {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < gainReductionBuffer.getNumSamples(); ++sample)
        {
            float reduction = juce::Decibels::gainToDecibels(channelData[sample]);
            if (reduction == -100) { continue; }
            maxReduction = std::min(maxReduction, reduction); // Track maximum reduction
        }
    }

    return maxReduction;
}


float Metrics::getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const
{
    // Ensure the buffer has valid samples
    if (gainReductionBuffer.getNumSamples() == 0 || gainReductionBuffer.getNumChannels() == 0)
        return 0.0f;

    float totalGainReduction = 0.0f;
    int totalSamples = 0;

    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
    {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < gainReductionBuffer.getNumSamples(); ++sample)
        {
            float reduction = juce::Decibels::gainToDecibels(channelData[sample]); // Convert gain to reduction
            if (reduction == -100) { continue; }
            totalGainReduction += std::fabs(reduction);   // Sum absolute reductions
        }
        totalSamples += gainReductionBuffer.getNumSamples();
    }

    return (totalSamples > 0) ? ((totalGainReduction / totalSamples)) : 0.0f;
}


float Metrics::getLRA(const juce::AudioBuffer<float>& buffer, double sampleRate, float windowDuration, float hopDuration) const
{
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return 0.0f;

    // Parameters for sliding window
    const int windowSize = static_cast<int>(windowDuration * sampleRate);
    const int hopSize = static_cast<int>(hopDuration * sampleRate);

    // Prepare K-Weighted Buffer
    juce::AudioBuffer<float> kWeightedBuffer;
    kWeightedBuffer.makeCopyOf(buffer);

    // init filters
    juce::dsp::IIR::Filter<float> lowShelfFilter, highShelfFilter;

    // Apply K-weighting filters
    lowShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, 1681.974450955533, 0.7071752369554196, 1.53512485958697);
    highShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 424.318677406412, 0.7071752369554196, 1.0);

    // Convert the buffer into a JUCE AudioBlock
    juce::dsp::AudioBlock<float> audioBlock(kWeightedBuffer);

    // Apply the filters channel by channel
    for (size_t channel = 0; channel < audioBlock.getNumChannels(); ++channel)
    {
        juce::dsp::ProcessContextReplacing<float> context(audioBlock.getSingleChannelBlock(channel));
        lowShelfFilter.process(context); // Apply the low-shelf filter
        highShelfFilter.process(context); // Apply the high-shelf filter
    }

    // Step 1: Calculate short-term loudness in sliding windows
    std::vector<float> shortTermLoudness;

    for (int startSample = 0; startSample + windowSize <= kWeightedBuffer.getNumSamples(); startSample += hopSize)
    {
        float sumSquares = 0.0f;

        for (int channel = 0; channel < kWeightedBuffer.getNumChannels(); ++channel)
        {
            const float* samples = kWeightedBuffer.getReadPointer(channel);
            for (int i = startSample; i < startSample + windowSize; ++i)
            {
                sumSquares += samples[i] * samples[i];
            }
        }

        int numSamplesInWindow = windowSize * kWeightedBuffer.getNumChannels();
        float rms = std::sqrt(sumSquares / numSamplesInWindow);

        // Convert RMS to LUFS for this window
        if (rms > 0.0f)
        {
            float lufs = 10.0f * std::log10(rms);
            shortTermLoudness.push_back(lufs);
        }
    }

    // Step 2: Calculate LRA from short-term loudness
    if (shortTermLoudness.empty())
        return 0.0f;

    // Sort loudness values to calculate percentiles
    std::sort(shortTermLoudness.begin(), shortTermLoudness.end());

    // Calculate 10th and 95th percentiles
    size_t n = shortTermLoudness.size();
    float lowPercentile = shortTermLoudness[static_cast<size_t>(0.1 * n)];
    float highPercentile = shortTermLoudness[static_cast<size_t>(0.95 * n)];

    return highPercentile - lowPercentile;
}

float Metrics::getGainReductionStdDev(const juce::AudioBuffer<float>& gainReductionBuffer)
{
    float sum = 0.0f;
    float mean = 0.0f;
    int totalSamples = gainReductionBuffer.getNumSamples();

    // Step 1: Compute the mean gain reduction
    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
    {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample)
        {
            mean += channelData[sample];
        }
    }
    mean /= (gainReductionBuffer.getNumChannels() * totalSamples);

    // Step 2: Compute the sum of squared deviations from the mean
    for (int channel = 0; channel < gainReductionBuffer.getNumChannels(); ++channel)
    {
        const float* channelData = gainReductionBuffer.getReadPointer(channel);
        for (int sample = 0; sample < totalSamples; ++sample)
        {
            float deviation = channelData[sample] - mean;
            sum += deviation * deviation;
        }
    }

    // Step 3: Return the standard deviation
    return std::sqrt(sum / (gainReductionBuffer.getNumChannels() * totalSamples));
}

float Metrics::getCompressionActivityRatio(const juce::AudioBuffer<float>* gainReductionSignal)
{
    // Check for signal validity
    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
    {
        DBG("Invalid or empty gain reduction signal.");
        return 0.0f;
    }

    /*DBG("Number of samples because i am a dumb motherfucker: " << gainReductionSignal->getNumSamples());*/

    const auto* data = gainReductionSignal->getReadPointer(0);
    int activeSamples = 0;

    DBG("Inspecting Gain Reduction Signal:");
    for (int i = 0; i < gainReductionSignal->getNumSamples(); ++i)
    {
        // Check activity using absolute value
        if (std::abs(data[i]) > 0.001f) // Use absolute value for comparison
            ++activeSamples;
    }

    if (activeSamples == 0)
    {
        DBG("No active samples found in gain reduction signal.");
    }

    return static_cast<float>(activeSamples) / static_cast<float>(gainReductionSignal->getNumSamples());
}

float Metrics::getGainReductionCrestFactor(const juce::AudioBuffer<float>* gainReductionSignal)
{
    // Check for signal validity
    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
        return 0.0f;

    const auto* data = gainReductionSignal->getReadPointer(0);
    int numSamples = gainReductionSignal->getNumSamples();

    // Find the peak and RMS values
    float peak = 0.0f;
    float rms = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        float absValue = std::abs(data[i]);
        peak = std::max(peak, absValue);
        rms += absValue * absValue;
    }

    rms = std::sqrt(rms / numSamples);

    // Avoid division by zero
    if (rms == 0.0f)
        return 0.0f;

    return peak / rms;
}

float Metrics::getDynamicRangeReduction(const juce::AudioBuffer<float>* uncompressedSignal,
    const juce::AudioBuffer<float>* compressedSignal) {
    if (uncompressedSignal == nullptr || compressedSignal == nullptr ||
        uncompressedSignal->getNumSamples() == 0 || compressedSignal->getNumSamples() == 0)
        return 0.0f;

    auto computeDynamicRange = [](const juce::AudioBuffer<float>* signal) {
        const float* data = signal->getReadPointer(0);
        int numSamples = signal->getNumSamples();
        float peak = *std::max_element(data, data + numSamples);
        float rms = std::sqrt(std::accumulate(data, data + numSamples, 0.0f,
            [](float sum, float sample) { return sum + sample * sample; }) / numSamples);
        return peak / rms; // Peak-to-RMS ratio
        };

    return computeDynamicRange(uncompressedSignal) - computeDynamicRange(compressedSignal);
}

float Metrics::getTransientImpact(const juce::AudioBuffer<float>* uncompressedSignal,
    const juce::AudioBuffer<float>* compressedSignal) {
    if (uncompressedSignal == nullptr || compressedSignal == nullptr ||
        uncompressedSignal->getNumSamples() == 0 || compressedSignal->getNumSamples() == 0)
        return 0.0f;

    auto computeTransientAmplitude = [](const juce::AudioBuffer<float>* signal) {
        const float* data = signal->getReadPointer(0);
        int numSamples = signal->getNumSamples();
        float maxAmplitude = 0.0f;
        for (int i = 1; i < numSamples; ++i) {
            maxAmplitude = std::max(maxAmplitude, std::abs(data[i] - data[i - 1]));
        }
        return maxAmplitude;
        };

    return computeTransientAmplitude(uncompressedSignal) - computeTransientAmplitude(compressedSignal);
}

float Metrics::getGainReductionRateOfChange(const juce::AudioBuffer<float>* gainReductionSignal, float sampleRate)
{
    // Check for signal validity
    if (!gainReductionSignal || gainReductionSignal->getNumSamples() < 2)
        return 0.0f;

    const auto* data = gainReductionSignal->getReadPointer(0);
    int numSamples = gainReductionSignal->getNumSamples();

    // Compute rate of change
    float sumRateOfChange = 0.0f;
    for (int i = 1; i < numSamples; ++i)
    {
        sumRateOfChange += std::abs(data[i] - data[i - 1]);
    }

    // Normalize to time
    return (sumRateOfChange / (numSamples - 1)) * sampleRate;
}

float Metrics::getGainReductionEnergy(const juce::AudioBuffer<float>* gainReductionSignal) {
    if (!gainReductionSignal || gainReductionSignal->getNumSamples() == 0)
        return 0.0f;

    const auto* data = gainReductionSignal->getReadPointer(0);
    float energy = 0.0f;

    for (int i = 0; i < gainReductionSignal->getNumSamples(); ++i) {
        energy += data[i] * data[i]; // Square each sample
    }

    return energy;
}

float Metrics::getRatioEfficiency(const juce::AudioBuffer<float>* inputSignal, const juce::AudioBuffer<float>* gainReductionSignal, float ratio) {
    if (!inputSignal || !gainReductionSignal ||
        inputSignal->getNumSamples() != gainReductionSignal->getNumSamples() || ratio <= 1.0f)
        return 0.0f;

    const auto* inputData = inputSignal->getReadPointer(0);
    const auto* grData = gainReductionSignal->getReadPointer(0);
    float efficiencySum = 0.0f;

    for (int i = 0; i < inputSignal->getNumSamples(); ++i) {
        float theoreticalReduction = (inputData[i] > 0.0f) ? (inputData[i] - inputData[i] / ratio) : 0.0f;
        efficiencySum += std::abs(grData[i] / theoreticalReduction);
    }

    return efficiencySum / inputSignal->getNumSamples();
}

float Metrics::getHarmonicDistortion(const juce::AudioBuffer<float>* inputSignal, const juce::AudioBuffer<float>* outputSignal, int fftOrder = 10)
{
    if (!inputSignal || !outputSignal ||
        inputSignal->getNumSamples() != outputSignal->getNumSamples())
        return 0.0f;

    // FFT setup
    const int fftSize = 1 << fftOrder; // FFT size (2^fftOrder)
    juce::dsp::FFT fft(fftOrder);

    if (inputSignal->getNumSamples() < fftSize || outputSignal->getNumSamples() < fftSize)
        return 0.0f;

    std::vector<float> inputFFTBuffer(fftSize * 2, 0.0f);  // Buffer for FFT input (real and imaginary parts)
    std::vector<float> outputFFTBuffer(fftSize * 2, 0.0f); // Buffer for FFT output (real and imaginary parts)

    // Copy input and output data into FFT buffers
    const auto* inputData = inputSignal->getReadPointer(0);
    const auto* outputData = outputSignal->getReadPointer(0);

    std::copy(inputData, inputData + fftSize, inputFFTBuffer.begin());
    std::copy(outputData, outputData + fftSize, outputFFTBuffer.begin());

    // Perform FFT on input and output signals
    fft.performRealOnlyForwardTransform(inputFFTBuffer.data());
    fft.performRealOnlyForwardTransform(outputFFTBuffer.data());

    // Calculate harmonic distortion
    float harmonicEnergy = 0.0f;
    float totalEnergy = 0.0f;

    for (int bin = 1; bin < fftSize / 2; ++bin) // Start at 1 to skip DC component
    {
        float inputMagnitude = std::sqrt(inputFFTBuffer[bin * 2] * inputFFTBuffer[bin * 2] + inputFFTBuffer[bin * 2 + 1] * inputFFTBuffer[bin * 2 + 1]);
        float outputMagnitude = std::sqrt(outputFFTBuffer[bin * 2] * outputFFTBuffer[bin * 2] + outputFFTBuffer[bin * 2 + 1] * outputFFTBuffer[bin * 2 + 1]);

        float harmonicDifference = outputMagnitude - inputMagnitude;

        harmonicEnergy += harmonicDifference * harmonicDifference; // Square the difference
        totalEnergy += outputMagnitude * outputMagnitude;          // Total energy in output
    }

    return (totalEnergy == 0.0f) ? 0.0f : harmonicEnergy / totalEnergy; // Return harmonic distortion as a ratio
}


//==============================================================================
bool Metrics::validateSignals(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const
{
    if (!signalA || !signalB)
    {
        DBG("One or both signals are null.");
        return false;
    }

    if (signalA->getNumSamples() != signalB->getNumSamples())
    {
        DBG("Signals must have the same number of samples.");
        return false;
    }

    return true;
}