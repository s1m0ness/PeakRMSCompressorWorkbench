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

    LoadedAudio out;
    out.sampleRate = reader->sampleRate;

    const int numChannels = (int)reader->numChannels;
    const int numSamples = (int)reader->lengthInSamples;

    out.buffer.setSize(numChannels, numSamples);
    reader->read(&out.buffer, 0, numSamples, 0, true, true);

    return out;
}
