#pragma once
#include <JuceHeader.h>

class DataExport
{
public:
    struct Config
    {
        juce::File baseDirectory;  // e.g. juce::File(Config::OutputPath::path)
        juce::String folderName = "PeakRMSCompressorWorkbench_testing_results";

        // WAV export settings
        int bitDepth = 24;

        // If false, engine can still call exportMetricsOnly().
        bool exportWavs = true;
    };

    explicit DataExport(Config cfg);

    // One-shot export entry point: ensures folder exists and writes everything.
    bool exportAll(const juce::File& inputFile,
        const juce::String& metricsText,
        const juce::AudioBuffer<float>* peakCompressed, // nullptr => skip
        const juce::AudioBuffer<float>* rmsCompressed,  // nullptr => skip
        double sampleRateForWav,                        // from loader
        juce::String* error = nullptr);

    // If you ever want only text, this is handy as hell.
    bool exportMetricsOnly(const juce::File& inputFile,
        const juce::String& metricsText,
        juce::String* error = nullptr);

    juce::File getOutputFolder() const { return outputFolder; }

private:
    bool ensureOutputFolder(juce::String* error);
    juce::File makeUniqueFile(const juce::File& inputFile,
        const juce::String& suffix,
        const juce::String& extension) const;

    bool saveText(const juce::File& file, const juce::String& content, juce::String* error) const;
    bool saveWav(const juce::File& file,
        const juce::AudioBuffer<float>& buffer,
        double sampleRate,
        int bitDepth,
        juce::String* error) const;

private:
    Config cfg;
    juce::File outputFolder;
};
