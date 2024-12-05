/*
 * Implementation based on the CTAGDRC project:
 * https://github.com/p-hlp/CTAGDRC
 *
 * This class implements an RMS Level Detector used in audio dynamic range processing.
 *
 * This file handles:
 * - Attack and release times adjustments controlling the degree of smoothing
 * - Applying peak-based detector smoothing to audio samples.
 *
 * NOTE: This implementation directly reuses parts of the original CTAGDRC code without modification.
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

class PeakLevelDetector
{
public:
    PeakLevelDetector() = default;

    // Prepares LevelDetector with a ProcessSpec-Object containing samplerate, blocksize and number of channels
    void prepare(const double& fs);

    // Sets attack time constant
    void setAttack(const double&);

    // Sets release time constant
    void setRelease(const double&);

    // Gets current attack time constant
    double getAttack();

    // Gets current release time constant
    double getRelease();

    // Gets calculated attack coefficient
    double getAlphaAttack();

    // gets calculated release coefficient
    double getAlphaRelease();

    // Processes a sample with smooth branched peak detector
    float processPeakBranched(const float&);

    // Applies ballistics to given buffer
    void applyPeakDetector(float*, int);


private:
    double attackTimeInSeconds{ 0.01 }, alphaAttack{ 0.0 };
    double releaseTimeInSeconds{ 0.14 }, alphaRelease{ 0.0 };
    double state01{ 0.0 }, state02{ 0.0 };
    double sampleRate{ 0.0 };
    bool autoAttack{ false };
    bool autoRelease{ false };
};
