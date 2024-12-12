/*
 * This file defines the constans used acrossed the project.
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
#include <cstdint>

namespace Constants
{
    namespace Colors
    {
        //Backgrounds
        constexpr uint32_t bg_App{ 0xff424242 };
        constexpr uint32_t bg_DarkGrey{ 0xff212121 };
        constexpr uint32_t bg_MidGrey{ 0xff616161 };
        constexpr uint32_t bg_LightGrey{ 0xff9e9e9e };

        //Gradients
        constexpr uint32_t gd_LightGreyStart{ 0xffeeeee };
        constexpr uint32_t gd_LightGreyEnd{ 0xffc4c4c4 };
        constexpr uint32_t gd_DarktGreyStart{ 0xff646464 };
        constexpr uint32_t gd_DarkGreyEnd{ 0xff464646 };

        //Misc
        constexpr uint32_t statusOutline{ 0xff00838f };
        constexpr uint32_t knobShadow{ 0xff424242 };
    }

    namespace Margins
    {
        constexpr float small = 2.0f;
        constexpr float medium = 4.0f;
        constexpr float big = 8.0f;
    }

    namespace Parameter
    {
        constexpr float thresholdStart = -60.0f;
        constexpr float thresholdEnd = 0.0f;
        constexpr float thresholdInterval = 0.1f;

        constexpr float ratioStart = 1.0f;
        constexpr float ratioEnd = 24.0f;
        constexpr float ratioInterval = 1.f;

        constexpr float kneeStart = 0.0f;
        constexpr float kneeEnd = 24.0f;
        constexpr float kneeInterval = 0.1f;

        constexpr float attackStart = 5.0f;
        constexpr float attackEnd = 500.0f;
        constexpr float attackInterval = 1.f;

        constexpr float releaseStart = 5.0f;
        constexpr float releaseEnd = 500.0f;
        constexpr float releaseInterval = 1.f;

        constexpr float makeupStart = -40.0f;
        constexpr float makeupEnd = 40.0f;
        constexpr float makeupInterval = 0.1f;
    }
}
