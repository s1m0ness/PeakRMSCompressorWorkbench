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

#include "include/PeakLevelDetector.h"
#include "../JuceLibraryCode/JuceHeader.h"

void PeakLevelDetector::prepare(const double& fs)
{
    sampleRate = fs;

    alphaAttack = exp(-1.0 / (sampleRate * attackTimeInSeconds));
    alphaRelease = exp(-1.0 / (sampleRate * releaseTimeInSeconds));
    state01 = 0.0;
    state02 = 0.0;
}

void PeakLevelDetector::setAttack(const double& attack)
{
    if (attack != attackTimeInSeconds)
    {
        attackTimeInSeconds = attack; //Time it takes to reach 1-1/e = 0.63
        alphaAttack = exp(-1.0 / (sampleRate * attackTimeInSeconds)); //aA = e^(-1/TA*fs)
    }
}

void PeakLevelDetector::setRelease(const double& release)
{
    if (release != releaseTimeInSeconds)
    {
        releaseTimeInSeconds = release; //Time it takes to reach 1 - (1-1/e) = 0.37
        alphaRelease = exp(-1.0 / (sampleRate * releaseTimeInSeconds)); //aR = e^(-1/TR*fs)
    }
}


double PeakLevelDetector::getAttack()
{
    return attackTimeInSeconds;
}

double PeakLevelDetector::getRelease()
{
    return releaseTimeInSeconds;
}

double PeakLevelDetector::getAlphaAttack()
{
    return alphaAttack;
}

double PeakLevelDetector::getAlphaRelease()
{
    return alphaRelease;
}

float PeakLevelDetector::processPeakBranched(const float& in)
{
    //Smooth branched peak detector
    if (in < state01) // since peak detector is placed after gain computer, the input values are negative
        state01 = alphaAttack * state01 + (1 - alphaAttack) * in;
    else
        state01 = alphaRelease * state01 + (1 - alphaRelease) * in;

    return static_cast<float>(state01); //y_L
}


void PeakLevelDetector::applyPeakDetector(float* src, int numSamples)
{
    // Apply smoothing to src buffer
    for (int i = 0; i < numSamples; ++i)
        src[i] = processPeakBranched(src[i]);

}