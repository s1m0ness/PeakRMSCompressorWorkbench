/*
 * This file defines the presets used for compressor's parameters.
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

namespace Preset {
    struct PresetData {
        int id;
        const char* name;
        struct Parameters {
            float threshold;
            float ratio;
            float attack;
            float release;
            float knee;
            float makeup;
        } peak, rms;
    };

    constexpr PresetData Drums = {
        1, "Drums",
        { -17.0f, 4.0f, 5.0f, 100.0f, 1.0f, 0.00f }, // Peak 
        { -19.5f, 4.0f, 5.0f, 100.0f, 1.0f, 0.00f }  // RMS 
    };

    constexpr PresetData Bass = {
        2, "Bass",
        { -20.0f, 4.0f, 20.0f, 60.0f, 2.0f, 0.00f }, // Peak
        { -24.0f, 4.0f, 20.0f, 60.0f, 2.0f, 0.00f }  // RMS
    };

    constexpr PresetData Guitar = {
        3, "Guitar",
        { -24.0f, 3.0f, 15.0f, 120.0f, 3.0f, 0.00f }, // Peak
        { -27.0f, 3.0f, 15.0f, 120.0f, 3.0f, 0.00f }  // RMS
    };

    constexpr PresetData Vocals = {
        4, "Vocals",
        { -14.0f, 4.0f, 5.0f, 150.0f, 6.0f, 0.00f }, // Peak
        { -17.0f, 4.0f, 5.0f, 150.0f, 6.0f, 0.00f }  // RMS
    };

    constexpr PresetData FullMix = {
        5,"FullMix",
        { -13.0f, 2.0f, 30.0f, 300.0f, 3.0f, 0.00f }, // Peak
        { -16.0f, 2.0f, 30.0f, 300.0f, 3.0f, 0.00f }  // RMS
    };

    //constexpr PresetData UserPreset = {
    //    6,"UserPreset",
    //    { -20.0f, 4.0f, 20.0f, 60.0f, 2.0f, 0.0f }, // Peak
    //    { -23.0f, 3.0f, 25.0f, 170.0f, 2.0f, 0.0f }  // RMS
    //};

    //constexpr PresetData UserPreset2 = {
    //    7,"UserPreset2",
    //    { -20.0f, 4.0f, 20.0f, 60.0f, 2.0f, 0.0f }, // Peak
    //    { -23.0f, 3.0f, 25.0f, 170.0f, 2.0f, 0.0f }  // RMS
    //};

    // add new presets to the list
    const std::initializer_list<PresetData> AllPresets = { 
        Drums, 
        Bass, 
        Guitar, 
        Vocals, 
        FullMix
    };
}

struct PresetStruct {
    std::string name;
    std::map<std::string, float> parameters;
};
