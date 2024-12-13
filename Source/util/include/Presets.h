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

namespace Presets
{
    namespace Drums {
        constexpr int ID = 1;
        constexpr const char* Name = "Drums";

        namespace Peak {
            constexpr float Threshold = -17.0f;
            constexpr float Ratio = 4.0f;
            constexpr float Attack = 5.0f;
            constexpr float Release = 100.0f;
            constexpr float Knee = 1.0f;
            constexpr float Makeup = 2.6f;
        }

        namespace RMS {
            constexpr float Threshold = -19.5f;
            constexpr float Ratio = 4.f;
            constexpr float Attack = 10.0f;
            constexpr float Release = 165.0f;
            constexpr float Knee = 1.0f;
            constexpr float Makeup = 2.4f;
        }
    }

    namespace Bass {
        constexpr int ID = 2;
        constexpr const char* Name = "Bass";

        namespace Peak {
            constexpr float Threshold = -20.f;
            constexpr float Ratio = 4.0f;
            constexpr float Attack = 20.0f;
            constexpr float Release = 60.0f;
            constexpr float Knee = 2.0f;
            constexpr float Makeup = 0.f;
        }

        namespace RMS {
            constexpr float Threshold = -23.0f;
            constexpr float Ratio = 3.0f;
            constexpr float Attack = 25.0f;
            constexpr float Release = 170.0f;
            constexpr float Knee = 2.0f;
            constexpr float Makeup = 0.f;
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
            constexpr float Makeup = 0.f;
        }

        namespace RMS {
            constexpr float Threshold = -20.0f;
            constexpr float Ratio = 2.8f;
            constexpr float Attack = 50.0f;
            constexpr float Release = 200.0f;
            constexpr float Knee = 2.5f;
            constexpr float Makeup = 0.f;
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
            constexpr float Makeup = 0.f;
        }

        namespace RMS {
            constexpr float Threshold = -14.0f;
            constexpr float Ratio = 3.7f;
            constexpr float Attack = 40.0f;
            constexpr float Release = 150.0f;
            constexpr float Knee = 1.8f;
            constexpr float Makeup = 0.f;
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
            constexpr float Makeup = 0.f;
        }

        namespace RMS {
            constexpr float Threshold = -12.0f;
            constexpr float Ratio = 2.2f;
            constexpr float Attack = 30.0f;
            constexpr float Release = 300.0f;
            constexpr float Knee = 1.5f;
            constexpr float Makeup = 0.f;
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
            constexpr float Makeup = 0.f;
        }

        namespace RMS {
            constexpr float Threshold = -12.0f;
            constexpr float Ratio = 2.2f;
            constexpr float Attack = 30.0f;
            constexpr float Release = 300.0f;
            constexpr float Knee = 1.5f;
            constexpr float Makeup = 0.f;
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
            constexpr float Makeup = 0.f;
        }

        namespace RMS {
            constexpr float Threshold = -12.0f;
            constexpr float Ratio = 2.2f;
            constexpr float Attack = 30.0f;
            constexpr float Release = 300.0f;
            constexpr float Knee = 1.5f;
            constexpr float Makeup = 0.f;
        }
    }*/
}


struct Preset {
    std::string name; // Name of the preset
    std::map<std::string, float> parameters; // Parameters of the preset
};

static const std::map<int, Preset> PresetParameters = {
    { Presets::Drums::ID,
      { Presets::Drums::Name,
        {
          {"peak_threshold", Presets::Drums::Peak::Threshold},
          {"peak_ratio",     Presets::Drums::Peak::Ratio},
          {"peak_attack",    Presets::Drums::Peak::Attack},
          {"peak_release",   Presets::Drums::Peak::Release},
          {"peak_knee",      Presets::Drums::Peak::Knee},
          {"peak_makeup",    Presets::Drums::Peak::Makeup},
          {"rms_threshold",  Presets::Drums::RMS::Threshold},
          {"rms_ratio",      Presets::Drums::RMS::Ratio},
          {"rms_attack",     Presets::Drums::RMS::Attack},
          {"rms_release",    Presets::Drums::RMS::Release},
          {"rms_knee",       Presets::Drums::RMS::Knee},
          {"rms_makeup",     Presets::Drums::RMS::Makeup}
        },
      }
    },
    { Presets::Bass::ID,
      { Presets::Bass::Name,
        {
          {"peak_threshold", Presets::Bass::Peak::Threshold},
          {"peak_ratio",     Presets::Bass::Peak::Ratio},
          {"peak_attack",    Presets::Bass::Peak::Attack},
          {"peak_release",   Presets::Bass::Peak::Release},
          {"peak_knee",      Presets::Bass::Peak::Knee},
          {"peak_makeup",    Presets::Bass::Peak::Makeup},
          {"rms_threshold",  Presets::Bass::RMS::Threshold},
          {"rms_ratio",      Presets::Bass::RMS::Ratio},
          {"rms_attack",     Presets::Bass::RMS::Attack},
          {"rms_release",    Presets::Bass::RMS::Release},
          {"rms_knee",       Presets::Bass::RMS::Knee},
          {"rms_makeup",     Presets::Bass::RMS::Makeup}
        },
      }
    },
    { Presets::Guitar::ID,
      { Presets::Guitar::Name,
        {
          {"peak_threshold", Presets::Guitar::Peak::Threshold},
          {"peak_ratio",     Presets::Guitar::Peak::Ratio},
          {"peak_attack",    Presets::Guitar::Peak::Attack},
          {"peak_release",   Presets::Guitar::Peak::Release},
          {"peak_knee",      Presets::Guitar::Peak::Knee},
          {"peak_makeup",    Presets::Guitar::Peak::Makeup},
          {"rms_threshold",  Presets::Guitar::RMS::Threshold},
          {"rms_ratio",      Presets::Guitar::RMS::Ratio},
          {"rms_attack",     Presets::Guitar::RMS::Attack},
          {"rms_release",    Presets::Guitar::RMS::Release},
          {"rms_knee",       Presets::Guitar::RMS::Knee},
          {"rms_makeup",     Presets::Guitar::RMS::Makeup}
        },
      }
    },
    { Presets::Vocals::ID,
      { Presets::Vocals::Name,
        {
          {"peak_threshold", Presets::Vocals::Peak::Threshold},
          {"peak_ratio",     Presets::Vocals::Peak::Ratio},
          {"peak_attack",    Presets::Vocals::Peak::Attack},
          {"peak_release",   Presets::Vocals::Peak::Release},
          {"peak_knee",      Presets::Vocals::Peak::Knee},
          {"peak_makeup",    Presets::Vocals::Peak::Makeup},
          {"rms_threshold",  Presets::Vocals::RMS::Threshold},
          {"rms_ratio",      Presets::Vocals::RMS::Ratio},
          {"rms_attack",     Presets::Vocals::RMS::Attack},
          {"rms_release",    Presets::Vocals::RMS::Release},
          {"rms_knee",       Presets::Vocals::RMS::Knee},
          {"rms_makeup",     Presets::Vocals::RMS::Makeup}
        },
      }
    },
    { Presets::FullMix::ID,
      { Presets::FullMix::Name,
        {
          {"peak_threshold", Presets::FullMix::Peak::Threshold},
          {"peak_ratio",     Presets::FullMix::Peak::Ratio},
          {"peak_attack",    Presets::FullMix::Peak::Attack},
          {"peak_release",   Presets::FullMix::Peak::Release},
          {"peak_knee",      Presets::FullMix::Peak::Knee},
          {"peak_makeup",    Presets::FullMix::Peak::Makeup},
          {"rms_threshold",  Presets::FullMix::RMS::Threshold},
          {"rms_ratio",      Presets::FullMix::RMS::Ratio},
          {"rms_attack",     Presets::FullMix::RMS::Attack},
          {"rms_release",    Presets::FullMix::RMS::Release},
          {"rms_knee",       Presets::FullMix::RMS::Knee},
          {"rms_makeup",     Presets::FullMix::RMS::Makeup}

        },
      }
    },

    // USER PRESETS

    /*{ Presets::UserPreset::ID,
      { Presets::UserPreset::Name,
        {
          {"peak_threshold", Presets::UserPreset::Peak::Threshold},
          {"peak_ratio",     Presets::UserPreset::Peak::Ratio},
          {"peak_attack",    Presets::UserPreset::Peak::Attack},
          {"peak_release",   Presets::UserPreset::Peak::Release},
          {"peak_knee",      Presets::UserPreset::Peak::Knee},
          {"peak_makeup",    Presets::userPreset::Peak::Makeup},
          {"rms_threshold",  Presets::UserPreset::RMS::Threshold},
          {"rms_ratio",      Presets::UserPreset::RMS::Ratio},
          {"rms_attack",     Presets::UserPreset::RMS::Attack},
          {"rms_release",    Presets::UserPreset::RMS::Release},
          {"rms_knee",       Presets::UserPreset::RMS::Knee},
          {"rms_makeup",     Presets::UserPreset::RMS::Makeup}
        },
      }
    },

    { Presets::UserPreset2::ID,
      { Presets::UserPreset2::Name,
        {
          {"peak_threshold", Presets::UserPreset2::Peak::Threshold},
          {"peak_ratio",     Presets::UserPreset2::Peak::Ratio},
          {"peak_attack",    Presets::UserPreset2::Peak::Attack},
          {"peak_release",   Presets::UserPreset2::Peak::Release},
          {"peak_knee",      Presets::UserPreset2::Peak::Knee},
          {"peak_makeup",    Presets::userPreset2::Peak::Makeup},
          {"rms_threshold",  Presets::UserPreset2::RMS::Threshold},
          {"rms_ratio",      Presets::UserPreset2::RMS::Ratio},
          {"rms_attack",     Presets::UserPreset2::RMS::Attack},
          {"rms_release",    Presets::UserPreset2::RMS::Release},
          {"rms_knee",       Presets::UserPreset2::RMS::Knee},
          {"rms_makeup",     Presets::UserPreset2::RMS::Makeup}
        },
      }
    },*/
};
