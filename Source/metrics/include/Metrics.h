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
        // Signals are in linear gain
        juce::AudioBuffer<float>* signal = nullptr;
        juce::AudioBuffer<float>* GRSignal = nullptr;
        
        char* signalName{ };
        bool isCompressed{ false };

        // Signal metrics
        float meanEnergy{ 0.0f };
        float rms{ 0.0f };
        float peak{ 0.0f };
        float crestFactor{ 0.0f };
        float lufs{ 0.0f };
        float lra{ 0.0f };

        // Metrics for comparing uncompressed and compressed signals
        float dynamicRangeReductionCrest{ 0.0f };
        float dynamicRangeReductionLRA{ 0.0f };
        float transientImpact{ 0.0f };
        float transientEnergyPreservation{ 0.0f };
        float harmonicDistortion{ 0.0f };
        
        // Gain reduction signal metrics
        float avgGR{ 0.0f };
        float maxGR{ 0.0f };
        float stdDevGR{ 0.0f };
        float energyGR{ 0.0f };
        float rateOfChangeGR = 0.0f;
        float compressionActivityRatio = 0.0f;
      
        juce::String formatMetrics() const
        {
            juce::String metricsContent;

            metricsContent << "**" << signalName << "**" << "\n";
            metricsContent << "-------------------\n";
            metricsContent << "Average energy in dB: " << juce::Decibels::gainToDecibels(meanEnergy) << "\n";
            metricsContent << "Peak value in dB: " << juce::Decibels::gainToDecibels(peak) << "\n";
            metricsContent << "RMS value in dB: " << juce::Decibels::gainToDecibels(rms) << "\n";
            metricsContent << "Crest factor in dB: " << crestFactor << "\n";
            metricsContent << "LUFS: " << lufs << "\n";
            metricsContent << "LRA: " << lra << "\n";
            
            if (isCompressed) {
                metricsContent << "\n";
                metricsContent << "Metrics for highliting compression effects:" << "\n";
                metricsContent << "-------------------\n";
                metricsContent << "Dynamic range reduction based on LRA in dB: " << dynamicRangeReductionLRA << "\n";
                metricsContent << "Dynamic range reduction based on crest factor in dB: " << dynamicRangeReductionCrest << "\n";
                metricsContent << "Transient impact: " << transientImpact << "\n";
                metricsContent << "Transient energy preservation: " << transientEnergyPreservation << "\n";
                metricsContent << "Total harmonic distortion: " << harmonicDistortion << "\n\n";
               
                metricsContent << "Gain reduction signal metrics:" << "\n";
                metricsContent << "-------------------\n";
                metricsContent << "Maximum gain reduction in dB: " << maxGR << "\n";
                metricsContent << "Average gain reduction in dB: " << avgGR << "\n";
                metricsContent << "Gain reduction energy in dB: " << energyGR << "\n";
                metricsContent << "Standard deviation of gain reduction: " << stdDevGR << "\n";
                metricsContent << "Rate of change of gain reduction: " << rateOfChangeGR << "\n";
                metricsContent << "Compression activity ratio: " << compressionActivityRatio << "\n";
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
    void extractMetrics();

private:
    //==============================================================================
    
    // Signal metrics
    //==============================================================================
    /**
     * Computes the peak value of the given audio buffer.
     *
     * @param buffer The input audio buffer.
     * @return The peak value in linear scale.
     */
    float getPeakValue(const juce::AudioBuffer<float>& buffer);

    /**
     * Calculates the average energy of the audio buffer.
     *
     * @param buffer The input audio buffer.
     * @return The average energy in linear scale.
     */
    float getAverageEnergy(const juce::AudioBuffer<float>& buffer);

    /**
     * Converts a mean square value into RMS.
     *
     * @param meanSquare The mean square value of the signal.
     * @return The RMS value in linear scale.
     */
    float getRMSValue(float meanSquare);

    /**
     * Computes the crest factor, which is the ratio of peak to RMS value.
     *
     * @param peakValue The peak value of the signal.
     * @param rmsValue The RMS value of the signal.
     * @return The crest factor in dB.
     */
    float getCrestFactor(float peakValue, float rmsValue);

    /**
     * Computes the Integrated Loudness (LUFS) of the given audio buffer, following
     * the EBU R128 standard.
     *
     * @param buffer The input audio buffer.
     * @return The LUFS value of the signal.
     */
    float getLUFS(const juce::AudioBuffer<float>& buffer);

    /**
     * Computes the Loudness Range (LRA) of the given audio buffer.
     *
     * LRA measures the dynamic range of loudness over time, following
     * the EBU R128 standard.
     *
     * @param buffer The input audio buffer.
     * @param sampleRate The sampling rate of the audio.
     * @param windowDuration The duration (in seconds) of each sliding window.
     * @param hopDuration The hop size (in seconds) between consecutive windows.
     * @return The Loudness Range (LRA) in LU (Loudness Units).
     */
    float getLRA(const juce::AudioBuffer<float>& buffer);

    // Metrics for comparing uncompressed and compressed signals
    //==============================================================================
    /**
     * Computes the reduction in crest factor due to compression.
     *
     * @param compressedCrestFactor The crest factor of the compressed signal.
     * @return The dynamic range reduction in dB.
     */
    float getDynamicRangeReductionCrest(float compressedCrestFactor);

    /**
     * Computes the reduction in Loudness Range (LRA) due to compression.
     *
     * @param compressedLRA The LRA of the compressed signal.
     * @return The dynamic range reduction in LU.
     */
    float getDynamicRangeReductionLRA(float compressedLRA);

    /**
     * Calculates the transient impact of compression on the signal.
     *
     * @param compressedSignal The compressed audio signal buffer.
     * @return The relative change in transient amplitude.
     */
    float getTransientImpact(const juce::AudioBuffer<float>& compressedSignal);

    /**
     * Measures how much transient energy is preserved after compression.
     *
     * @param compressedSignal The compressed audio signal buffer.
     * @return The transient energy preservation ratio (0 to 1).
     */
    float getTransientEnergyPreservation(const juce::AudioBuffer<float>& compressedSignal);

    /**
     * Computes the harmonic distortion introduced by compression.
     *
     * @param compressedBuffer The compressed audio signal buffer.
     * @return The harmonic distortion in dB.
     */
    float getWaveformDistortion(const juce::AudioBuffer<float>& compressedBuffer);

    // Gain reduction signal metrics
    //==============================================================================
    /**
     * Finds the maximum gain reduction applied during compression.
     *
     * @param gainReductionBuffer The buffer representing the gain reduction signal.
     * @return The maximum gain reduction in dB.
     */
    float getMaxGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const;

    /**
     * Computes the average gain reduction applied during compression.
     *
     * @param gainReductionBuffer The buffer representing the gain reduction signal.
     * @return The average gain reduction in dB.
     */
    float getAverageGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer) const;

    /**
     * Computes the standard deviation of the gain reduction signal.
     *
     * @param gainReductionBuffer The buffer representing the gain reduction signal.
     * @param mean The mean value of gain reduction.
     * @return The standard deviation of the gain reduction in dB.
     */
    float getStdDevGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer, float mean);

    /**
     * Measures the total energy reduction due to compression.
     *
     * @param gainReductionBuffer The buffer representing the gain reduction signal.
     * @return The gain reduction energy in dB.
     */
    float getEnergyGainReduction(const juce::AudioBuffer<float>& gainReductionBuffer);

    /**
     * Calculates the rate of change in the gain reduction signal.
     *
     * @param gainReductionSignal The buffer representing the gain reduction signal.
     * @return The average rate of change of gain reduction.
     */
    float getRateOfChangeGainReduction(const juce::AudioBuffer<float>& gainReductionSignal);

    /**
     * Computes the ratio of time the compressor is actively reducing gain.
     *
     * @param gainReductionSignal The buffer representing the gain reduction signal.
     * @return The compression activity ratio (0 to 1).
     */
    float getCompressionActivityRatio(const juce::AudioBuffer<float>& gainReductionSignal);

    
    
    //==============================================================================
    bool validateSignals() const;
   
    void applyKWeighting(juce::AudioBuffer<float>& buffer);

    std::vector<juce::AudioBuffer<float>> getWindowsFromBuffer(const juce::AudioBuffer<float>& buffer);

    //==============================================================================

    float getIntegratedLoudness(float meanSquare);

    std::vector<float> getShortTermLoudness(juce::AudioBuffer<float>& buffer);
    
    //==============================================================================
    float transientPercentile = 0.5f;
    double sampleRate{ 0.0f };

    // for extracting windows for short-term calculations
    float windowDuration{ 0.4f }; // 40 ms windows
    float hopDuration{ 0.2f }; // 20 ms overlaps

    CompressionMetrics uncompressedMetrics;
    CompressionMetrics peakMetrics;
    CompressionMetrics rmsMetrics;
};
