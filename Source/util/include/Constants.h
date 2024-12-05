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

        /*constexpr std::array<double, 14> ratioChoices = { 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100 };*/
    }

    namespace OutputPath
    {
        // Configure the path to your liking
        constexpr char outputPath[] = "C:/Users/Public/Documents/";
    }

    namespace Presets {

        namespace Drums {
            constexpr int ID = 1;
            constexpr const char* Name = "Drums";

            namespace Peak {
                constexpr float Threshold = -17.0f;
                constexpr float Ratio = 4.0f;
                constexpr float Attack = 5.0f;
                constexpr float Release = 100.0f;
                constexpr float Knee = 1.0f;
            }

            namespace RMS {
                constexpr float Threshold = -12.0f;
                constexpr float Ratio = 3.5f;
                constexpr float Attack = 5.0f;
                constexpr float Release = 120.0f;
                constexpr float Knee = 1.5f;
            }
        }

        namespace Bass {
            constexpr int ID = 2;
            constexpr const char* Name = "Bass";

            namespace Peak {
                constexpr float Threshold = -15.0f;
                constexpr float Ratio = 3.0f;
                constexpr float Attack = 60.0f;
                constexpr float Release = 300.0f;
                constexpr float Knee = 1.5f;
            }

            namespace RMS {
                constexpr float Threshold = -17.0f;
                constexpr float Ratio = 3.0f;
                constexpr float Attack = 70.0f;
                constexpr float Release = 350.0f;
                constexpr float Knee = 1.8f;
            }
        }

        namespace Guitar {
            constexpr int ID = 3;
            constexpr const char* Name = "Guitar";

            namespace Peak {
                constexpr float Threshold = -18.0f;
                constexpr float Ratio = 2.5f;
                constexpr float Attack = 40.0f;
                constexpr float Release = 150.0f;
                constexpr float Knee = 2.0f;
            }

            namespace RMS {
                constexpr float Threshold = -20.0f;
                constexpr float Ratio = 2.8f;
                constexpr float Attack = 50.0f;
                constexpr float Release = 200.0f;
                constexpr float Knee = 2.5f;
            }
        }

        namespace Vocals {
            constexpr int ID = 4;
            constexpr const char* Name = "Vocals";

            namespace Peak {
                constexpr float Threshold = -12.0f;
                constexpr float Ratio = 3.5f;
                constexpr float Attack = 30.0f;
                constexpr float Release = 100.0f;
                constexpr float Knee = 1.2f;
            }

            namespace RMS {
                constexpr float Threshold = -14.0f;
                constexpr float Ratio = 3.7f;
                constexpr float Attack = 40.0f;
                constexpr float Release = 150.0f;
                constexpr float Knee = 1.8f;
            }
        }

        namespace FullMix {
            constexpr int ID = 5;
            constexpr const char* Name = "Full Mix";

            namespace Peak {
                constexpr float Threshold = -10.0f;
                constexpr float Ratio = 2.0f;
                constexpr float Attack = 20.0f;
                constexpr float Release = 250.0f;
                constexpr float Knee = 1.0f;
            }

            namespace RMS {
                constexpr float Threshold = -12.0f;
                constexpr float Ratio = 2.2f;
                constexpr float Attack = 30.0f;
                constexpr float Release = 300.0f;
                constexpr float Knee = 1.5f;
            }
        }

        // USER PRESETS
        
        /*namespace UserPreset {
            constexpr int ID = 6;
            constexpr const char* Name = "User Preset";

            namespace Peak {
                constexpr float Threshold = -10.0f;
                constexpr float Ratio = 2.0f;
                constexpr float Attack = 20.0f;
                constexpr float Release = 250.0f;
                constexpr float Knee = 1.0f;
            }

            namespace RMS {
                constexpr float Threshold = -12.0f;
                constexpr float Ratio = 2.2f;
                constexpr float Attack = 30.0f;
                constexpr float Release = 300.0f;
                constexpr float Knee = 1.5f;
            }
        }

        namespace UserPreset2 {
            constexpr int ID = 6;
            constexpr const char* Name = "User Preset 2";

            namespace Peak {
                constexpr float Threshold = -10.0f;
                constexpr float Ratio = 2.0f;
                constexpr float Attack = 20.0f;
                constexpr float Release = 250.0f;
                constexpr float Knee = 1.0f;
            }

            namespace RMS {
                constexpr float Threshold = -12.0f;
                constexpr float Ratio = 2.2f;
                constexpr float Attack = 30.0f;
                constexpr float Release = 300.0f;
                constexpr float Knee = 1.5f;
            }
        }*/
        
    }
}

struct Preset {
    std::string name; // Name of the preset
    std::map<std::string, float> parameters; // Parameters of the preset
};

static const std::map<int, Preset> PresetParameters = {
    { Constants::Presets::Drums::ID, 
      { Constants::Presets::Drums::Name,
        { 
          {"peak_threshold", Constants::Presets::Drums::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::Drums::Peak::Ratio},
          {"peak_attack",    Constants::Presets::Drums::Peak::Attack},
          {"peak_release",   Constants::Presets::Drums::Peak::Release},
          {"peak_knee",      Constants::Presets::Drums::Peak::Knee},
          {"rms_threshold",  Constants::Presets::Drums::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::Drums::RMS::Ratio},
          {"rms_attack",     Constants::Presets::Drums::RMS::Attack},
          {"rms_release",    Constants::Presets::Drums::RMS::Release},
          {"rms_knee",       Constants::Presets::Drums::RMS::Knee}
        },
      }
    },
    { Constants::Presets::Bass::ID, 
      { Constants::Presets::Bass::Name,
        { 
          {"peak_threshold", Constants::Presets::Bass::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::Bass::Peak::Ratio},
          {"peak_attack",    Constants::Presets::Bass::Peak::Attack},
          {"peak_release",   Constants::Presets::Bass::Peak::Release},
          {"peak_knee",      Constants::Presets::Bass::Peak::Knee},
          {"rms_threshold",  Constants::Presets::Bass::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::Bass::RMS::Ratio},
          {"rms_attack",     Constants::Presets::Bass::RMS::Attack},
          {"rms_release",    Constants::Presets::Bass::RMS::Release},
          {"rms_knee",       Constants::Presets::Bass::RMS::Knee}
        },
      }
    },
    { Constants::Presets::Guitar::ID, 
      { Constants::Presets::Guitar::Name,
        { 
          {"peak_threshold", Constants::Presets::Guitar::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::Guitar::Peak::Ratio},
          {"peak_attack",    Constants::Presets::Guitar::Peak::Attack},
          {"peak_release",   Constants::Presets::Guitar::Peak::Release},
          {"peak_knee",      Constants::Presets::Guitar::Peak::Knee},
          {"rms_threshold",  Constants::Presets::Guitar::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::Guitar::RMS::Ratio},
          {"rms_attack",     Constants::Presets::Guitar::RMS::Attack},
          {"rms_release",    Constants::Presets::Guitar::RMS::Release},
          {"rms_knee",       Constants::Presets::Guitar::RMS::Knee}
        },
      }
    },
    { Constants::Presets::Vocals::ID, 
      { Constants::Presets::Vocals::Name,
        { 
          {"peak_threshold", Constants::Presets::Vocals::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::Vocals::Peak::Ratio},
          {"peak_attack",    Constants::Presets::Vocals::Peak::Attack},
          {"peak_release",   Constants::Presets::Vocals::Peak::Release},
          {"peak_knee",      Constants::Presets::Vocals::Peak::Knee},
          {"rms_threshold",  Constants::Presets::Vocals::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::Vocals::RMS::Ratio},
          {"rms_attack",     Constants::Presets::Vocals::RMS::Attack},
          {"rms_release",    Constants::Presets::Vocals::RMS::Release},
          {"rms_knee",       Constants::Presets::Vocals::RMS::Knee}
        },
      }
    },
    { Constants::Presets::FullMix::ID, 
      { Constants::Presets::FullMix::Name,
        { 
          {"peak_threshold", Constants::Presets::FullMix::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::FullMix::Peak::Ratio},
          {"peak_attack",    Constants::Presets::FullMix::Peak::Attack},
          {"peak_release",   Constants::Presets::FullMix::Peak::Release},
          {"peak_knee",      Constants::Presets::FullMix::Peak::Knee},
          {"rms_threshold",  Constants::Presets::FullMix::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::FullMix::RMS::Ratio},
          {"rms_attack",     Constants::Presets::FullMix::RMS::Attack},
          {"rms_release",    Constants::Presets::FullMix::RMS::Release},
          {"rms_knee",       Constants::Presets::FullMix::RMS::Knee}
        },
      }
    },

    // USER PRESETS

    /*{ Constants::Presets::UserPreset::ID, 
      { Constants::Presets::UserPreset::Name,
        { 
          {"peak_threshold", Constants::Presets::UserPreset::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::UserPreset::Peak::Ratio},
          {"peak_attack",    Constants::Presets::UserPreset::Peak::Attack},
          {"peak_release",   Constants::Presets::UserPreset::Peak::Release},
          {"peak_knee",      Constants::Presets::UserPreset::Peak::Knee},
          {"rms_threshold",  Constants::Presets::UserPreset::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::UserPreset::RMS::Ratio},
          {"rms_attack",     Constants::Presets::UserPreset::RMS::Attack},
          {"rms_release",    Constants::Presets::UserPreset::RMS::Release},
          {"rms_knee",       Constants::Presets::UserPreset::RMS::Knee}
        },
      }
    },

    { Constants::Presets::UserPreset2::ID,
      { Constants::Presets::UserPreset2::Name,
        {
          {"peak_threshold", Constants::Presets::UserPreset2::Peak::Threshold},
          {"peak_ratio",     Constants::Presets::UserPreset2::Peak::Ratio},
          {"peak_attack",    Constants::Presets::UserPreset2::Peak::Attack},
          {"peak_release",   Constants::Presets::UserPreset2::Peak::Release},
          {"peak_knee",      Constants::Presets::UserPreset2::Peak::Knee},
          {"rms_threshold",  Constants::Presets::UserPreset2::RMS::Threshold},
          {"rms_ratio",      Constants::Presets::UserPreset2::RMS::Ratio},
          {"rms_attack",     Constants::Presets::UserPreset2::RMS::Attack},
          {"rms_release",    Constants::Presets::UserPreset2::RMS::Release},
          {"rms_knee",       Constants::Presets::UserPreset2::RMS::Knee}
        },
      }
    },*/
};
