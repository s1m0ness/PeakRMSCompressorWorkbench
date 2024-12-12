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
 * NOTE: This implementation directly reuses the original CTAGDRC code for gain computer.
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

class GainComputer
{
public:

    GainComputer();

    // Sets the threshold in dB
    void setThreshold(float db);

    // Sets the ratio in dB
    void setRatio(float db);

    //sets the knee in db (if > 0, 2nd order interpolation for soft knee)
    void setKnee(float db);

    // Applies characteristics to a given sample
    // returns attenuation
    float applyCompression(float&);

    void applyCompressionToBuffer(float*, int);

    float threshold{ -20.0f };
    float ratio{ 2.0f };
private:
    float knee{ 6.0f }, kneeHalf{ 3.0f };
    float slope{ -0.5f };
};
