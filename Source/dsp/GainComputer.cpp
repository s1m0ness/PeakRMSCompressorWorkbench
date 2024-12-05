/*
 * Implementation based on the CTAGDRC project:
 * https://github.com/p-hlp/CTAGDRC
 *
 * This class, GainComputer, handles gain calculation for dynamic range compression.
 * The logic used here is derived from the CTAGDRC project, specifically for threshold, ratio,
 * and knee-based gain reduction. All calculations are in alignment with digital audio standards
 * for smooth compression behavior, including soft-knee handling.
 *
 * This file handles:
 * - Threshold adjustments for compression onset.
 * - Ratio-based gain reduction.
 * - Soft-knee computations for smoother compression.
 * - Applying gain reduction to audio buffers for real-time processing.
 *
 * NOTE: This implementation directly reuses the original CTAGDRC code for gain computer without modification.
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

#include "include/GainComputer.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include "../JuceLibraryCode/JuceHeader.h"

GainComputer::GainComputer()
{
    threshold = -20.0f;
    ratio = 2.0f;
    slope = 1.0f / ratio - 1.0f;
    knee = 6.0f;
    kneeHalf = 3.0f;
}

void GainComputer::setThreshold(float newTreshold)
{
    threshold = newTreshold;
}

void GainComputer::setRatio(float newRatio)
{
    if (ratio != newRatio)
    {
        ratio = newRatio;
        if (ratio > 23.9f) ratio = -std::numeric_limits<float>::infinity();
        slope = 1.0f / newRatio - 1.0f;
    }
}

void GainComputer::setKnee(float newKnee)
{
    if (newKnee != knee)
    {
        knee = newKnee;
        kneeHalf = newKnee / 2.0f;
    }
}

float GainComputer::applyCompression(float& input)
{
    const float overshoot = input - threshold;

    if (overshoot <= -kneeHalf)
        return 0.0f;
    if (overshoot > -kneeHalf && overshoot <= kneeHalf)
        return 0.5f * slope * ((overshoot + kneeHalf) * (overshoot + kneeHalf)) / knee;


    return slope * overshoot;
}

void GainComputer::applyCompressionToBuffer(float* src, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float level = std::max(abs(src[i]), 1e-6f);
        float levelInDecibels = juce::Decibels::gainToDecibels(level);
        src[i] = applyCompression(levelInDecibels);
    }
}
