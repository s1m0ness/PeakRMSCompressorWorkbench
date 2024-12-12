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

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <string>

// Metrics class to calculate and store compression metrics
class Metrics
{
public:
    //==============================================================================
    Metrics();
    ~Metrics();

    //==============================================================================
    struct CompressionMetrics
    {
        /*float rmse = 0.0f;
        float correlation = 0.0f;
        float LRA = 0.0f;
        float stdDevGR = 0.0f;
        float compressionActivity = 0.0f;
        float crestGR = 0.0f;
        float transientImpact = 0.0f;
        float rateOfChangeGR = 0.0f;
        float gainReductionEnergy = 0.0f;
        float ratioEfficiency = 0.0f;
        float attackReleaeTime = 0.0f;
        float temporalSmoothness = 0.0f;
        float transientEnergyPreservation = 0.0f;*/

        float meanSquare{ 0.0f };
        float rms{ 0.0f };
        float peak{ 0.0f };
        float crestFactor{ 0.0f };
        float lufs{ 0.0f };
        float dynamicRangeReduction{ 0.0f };
        float avgGR{ 0.0f };
        float maxGR{ 0.0f };
        
        char* signalName{ };
        bool trackGR{ false };

        juce::String formatMetrics() const
        {
            juce::String metricsContent;

            metricsContent << signalName << "\n";
            metricsContent << "-------------------\n";
            metricsContent << "Peak value in dB: " << juce::Decibels::gainToDecibels(peak) << "\n";
            metricsContent << "Mean square energy in dB: " << juce::Decibels::gainToDecibels(meanSquare) << "\n";
            metricsContent << "RMS value in dB: " << juce::Decibels::gainToDecibels(rms) << "\n";
            metricsContent << "Crest factor in dB: " << crestFactor << "\n";
            metricsContent << "LUFS: " << lufs << "\n";
            
            if (trackGR) {
                metricsContent << "Maximum gain reduction in dB: " << maxGR << "\n";
                metricsContent << "Average gain reduction in dB: " << avgGR << "\n";
            }
            metricsContent << "\n";
            return metricsContent;
        }
    };

    //==============================================================================
    const CompressionMetrics& getUncompressedMetrics() const;
    const CompressionMetrics& getPeakMetrics() const;
    const CompressionMetrics& getRMSMetrics() const;

    //==============================================================================
    void prepare(const double& fs);

    //==============================================================================
    void setUncompressedSignal(juce::AudioBuffer<float>* signal);
    
    void setPeakCompressedSignal(juce::AudioBuffer<float>* signal);
    
    void setPeakGainReductionSignal(juce::AudioBuffer<float>* signal);
    
    void setRMSCompressedSignal(juce::AudioBuffer<float>* signal);

    void setRMSGainReductionSignal(juce::AudioBuffer<float>* signal);


    //==============================================================================
    void extractMetrics(float peakThreshold, float rmsThreshold);

private:
    //==============================================================================
    float getMeanSquareEnergy(const juce::AudioBuffer<float>& signal);
    
    float getPeakValue(const juce::AudioBuffer<float>& signal);

    float getRMSValue(float meanSquare);

    float getCrestFactor(float peakValue, float rmsValue);

    float getLUFS(const juce::AudioBuffer<float>& buffer);

    float getDynamicRangeReduction(bool isRMS);





    float getRMSE(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const;

    float getCorrelation(const juce::AudioBuffer<float>* signalA, const juce::AudioBuffer<float>* signalB) const;

    float getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const;

    float getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const;
    
    float getCrestFactor(const juce::AudioBuffer<float>& buffer) const;

    /*
    * Computes the Loudness Range (LRA) of the given audio buffer.
    *
    * LRA is calculated following the EBU R128 standard, which measures
    * the dynamic range of loudness over time. The LRA is derived from
    * the difference between the 95th and 10th percentiles of short-term
    * loudness values, calculated over sliding windows.
    *
    * Steps:
    * 1. Apply K-weighting filters to simulate human hearing sensitivity.
    * 2. Use a sliding window approach to calculate short-term loudness (LUFS).
    * 3. Sort short-term loudness values and exclude outliers (extreme values).
    * 4. Calculate the difference between the 95th and 10th percentiles.
    *
    * @param buffer The input audio buffer.
    * @param sampleRate The sampling rate of the audio.
    * @param windowDuration The duration (in seconds) of each sliding window.
    * @param hopDuration The hop size (in seconds) between consecutive windows.
    * @return The Loudness Range (LRA) in LU (Loudness Units).
    */
    float getLRA(const juce::AudioBuffer<float>& buffer, double sampleRate, float windowDuration, float hopDuration) const;

    float getGainReductionStdDev(const juce::AudioBuffer<float>& gainReductionBuffer);

    float getCompressionActivityRatio(const juce::AudioBuffer<float>* gainReductionSignal);

    float getGainReductionCrestFactor(const juce::AudioBuffer<float>* gainReductionSignal);

    float getTransientImpact(const juce::AudioBuffer<float>* uncompressedSignal, const juce::AudioBuffer<float>* compressedSignal);

    float getGainReductionRateOfChange(const juce::AudioBuffer<float>* gainReductionSignal, float sampleRate);

    float getGainReductionEnergy(const juce::AudioBuffer<float>* gainReductionSignal);

    float getRatioEfficiency(const juce::AudioBuffer<float>* inputSignal, const juce::AudioBuffer<float>* gainReductionSignal, float ratio);

    float getAttackReleaseTime(const juce::AudioBuffer<float>* gainReductionSignal, float sampleRate);

    float getTemporalSmoothness(const juce::AudioBuffer<float>* gainReductionSignal);

    float getTransientEnergyPreservation(const juce::AudioBuffer<float>* uncompressedSignal, const juce::AudioBuffer<float>* compressedSignal);


    //==============================================================================
    bool validateSignal(const juce::AudioBuffer<float>& signal) const;

    void downsampleBuffer(juce::AudioBuffer<float>* buffer);

    //==============================================================================
    void applyKWeighting(juce::AudioBuffer<float>& buffer);

    float getIntegratedLoudness(float meanSquare);


    
    // All signals values are in linear gain
    //==============================================================================
    juce::AudioBuffer<float>* uncompressedSignal = nullptr;
    juce::AudioBuffer<float>* peakCompressedSignal = nullptr;
    juce::AudioBuffer<float>* rmsCompressedSignal = nullptr;
    juce::AudioBuffer<float>* rmsGainReductionSignal = nullptr;
    juce::AudioBuffer<float>* peakGainReductionSignal = nullptr;
    
    double sampleRate{ 0.0f };
    int downSamplingFactor{ 10 };

    CompressionMetrics uncompressedMetrics;
    CompressionMetrics peakMetrics;
    CompressionMetrics rmsMetrics;
};
