/*
 * Implementation based on the CTAGDRC project:
 * https://github.com/p-hlp/CTAGDRC
 *
 * This class implements a peak envelope follower that tracks the peak levels
 * of an audio signal over time.

 * This file handles:
 * - Updates the peak value based on audio input samples.
 * - Returns the current peak level.
 *
 * NOTE: This implementation directly reuses CTAGDRC code for peak level envlope follower without modification.
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

#include "include/LevelEnvelopeFollower.h"
#include <cassert>
#include <cmath>

void LevelEnvelopeFollower::prepare(const double& fs)
{
    sampleRate = fs;

    peakDecayInSamples = static_cast<int>(peakDecayInSeconds * sampleRate);
    peakDecay = 1.0f - 1.0f / static_cast<float>(peakDecayInSamples);
}

void LevelEnvelopeFollower::setPeakDecay(float dc)
{
    peakDecayInSeconds = dc;
    prepare(sampleRate);
}

// Updates peak envelope follower from given audio buffer
void LevelEnvelopeFollower::updatePeak(const float* const* channelData, int numChannels, int numSamples)
{
    assert(numChannels >= 0 && numSamples >= 0 && channelData != nullptr);
    if (numChannels > 0 && numSamples > 0)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float sum = 0.0f;
            for (int j = 0; j < numChannels; ++j)
                sum += std::abs(channelData[j][i]);

            sum /= static_cast<float>(numChannels);

            if (sum > currMaxPeak)
                currMaxPeak = sum;
            else if (currMaxPeak > 0.001f)
                currMaxPeak *= peakDecay;
            else currMaxPeak = 0.0f;
        }
    }
}

// Gets current peak, call after updatePeak
float LevelEnvelopeFollower::getPeak()
{
    return currMaxPeak;
}