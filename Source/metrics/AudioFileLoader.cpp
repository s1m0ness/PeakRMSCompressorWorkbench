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
#include "include/AudioFileLoader.h"

juce::File AudioFileLoader::chooseAudioFile()
{
    juce::FileChooser chooser(
        "Select the audio file to analyze...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.mp3"
    );

    if (!chooser.browseForFileToOpen())
        return {}; // empty = cancelled

    auto f = chooser.getResult();
    if (!f.existsAsFile())
        return {};

    return f;
}

std::optional<AudioFileLoader::LoadedAudio>
AudioFileLoader::loadAudioFile(const juce::File& file, juce::String* error) const
{
    if (!file.existsAsFile())
    {
        if (error) *error = "File does not exist.";
        return std::nullopt;
    }

    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
    if (!reader)
    {
        if (error) *error = "Unable to create reader.";
        return std::nullopt;
    }

    // Enforce mono or stereo only
    if (reader->numChannels < 1 || reader->numChannels > 2)
    {
        if (error)
            *error = "Only mono and stereo audio files are supported.";
        return std::nullopt;
    }

    LoadedAudio out;
    out.sampleRate = reader->sampleRate;

    const int numChannels = (int)reader->numChannels;
    const int numSamples = (int)reader->lengthInSamples;

    out.buffer.setSize(numChannels, numSamples);
    reader->read(&out.buffer, 0, numSamples, 0, true, true);

    return out;
}
