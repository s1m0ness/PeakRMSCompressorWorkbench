/*
 * This file defines the output configuration for the project.
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

namespace Config
{
    namespace OutputPath
    {
        // Detect operating system and configure the output path accordingly
#ifdef _WIN32
        constexpr char path[] = "C:/Users/Public/Documents"; // Windows default path
#elif defined(__APPLE__)
        constexpr char path[] = "/Users/Shared/"; // macOS default path
#else
        constexpr char path[] = "./"; // Fallback for other OS
#endif
    }

    namespace saveCompressedFiles
    {
        // Do you wish to also save both peak and rms compressed files?
        constexpr bool save = false;
    }
}
