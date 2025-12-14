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

    // UI: choose file (call on message thread)
    static juce::File chooseAudioFile();

    // I/O: load file (can be called on background thread)
    std::optional<LoadedAudio> loadAudioFile(const juce::File& file, juce::String* error = nullptr) const;

private:
    juce::AudioFormatManager& formatManager;
};

