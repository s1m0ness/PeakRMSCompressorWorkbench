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

#pragma once

/* Basic envelope-follwer, to track peak & rms signal level with configurable decay time*/
class LevelEnvelopeFollower
{
public:
    LevelEnvelopeFollower() = default;

    // Prepares envelope follower with given sample rate and recalculates decayInSamples
    // as well as the peak/rms coefficient
    void prepare(const double& fs);

    // Set peak decay
    void setPeakDecay(float dc);

    // Updates peak envelope follower from given audio buffer
    void updatePeak(const float* const* channelData, int numChannels, int numSamples);

    // Gets current peak, call after updatePeak
    float getPeak();

private:
    float currMaxPeak{ 0.0f };
    float peakDecay{ 0.99992f };
    float peakDecayInSeconds{ 0.5f };

    int peakDecayInSamples{ 0 };

    double sampleRate{ 0.0f };
};