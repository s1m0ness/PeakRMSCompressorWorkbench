/*
 * This file defines the AudioFileLoader class, responsible for loading audio
 * files used by the experimental workbench.
 *
 * Key Features:
 * - Provides a user-facing file selection mechanism.
 * - Loads audio files into memory buffers for further processing.
 * - Extracts and stores the associated sample rate information.
 * - Supports error reporting during file loading.
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
#include <JuceHeader.h>

class AudioFileLoader
{
public:
    struct LoadedAudio
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 0.0;
    };

    explicit AudioFileLoader(juce::AudioFormatManager& fm) : formatManager(fm) {}

    /**
    * Opens a file chooser dialog allowing the user to select an audio file.
    *
    * This function is intended to be called from the message (UI) thread (PluginEditor).
    *
    * @return The selected audio file, or an invalid file if selection was cancelled.
    */
    static juce::File chooseAudioFile();

    /**
     * Loads an audio file into memory.
     *
     * The audio data is decoded into a floating-point buffer, and the
     * corresponding sample rate is extracted. This function can be safely
     * called from a background thread.
     *
     * @param file The audio file to be loaded.
     * @param error Optional pointer for returning an error message.
     * @return An optional LoadedAudio structure containing the audio buffer
     *         and sample rate, or std::nullopt if loading failed.
     */
    std::optional<LoadedAudio> loadAudioFile(const juce::File& file, juce::String* error = nullptr) const;

private:
    juce::AudioFormatManager& formatManager;
};

