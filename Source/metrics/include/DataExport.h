/*
 * This file defines the DataExport class, responsible for exporting analysis
 * results produced by the experimental workbench.
 *
 * Key Features:
 * - Exports objective evaluation metrics as text files.
 * - Optionally exports compressed audio signals as WAV files.
 * - Manages output directories and file naming to avoid overwriting results.
 * - Supports configurable export settings such as bit depth and output paths.
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
        bool exportWavs;  // e.g. Config::saveCompressedFiles::save
    };

    /**
     * Constructs a DataExport instance with the given configuration.
     *
     * @param cfg Configuration structure specifying output paths and export options.
     */
    explicit DataExport(Config cfg);

    /**
     * Exports all available analysis results for a given input file.
     *
     * This function ensures that the output directory exists, writes the
     * metrics text file, and optionally exports compressed audio signals
     * as WAV files.
     *
     * @param inputFile The original input audio file.
     * @param metricsText Textual representation of the computed metrics.
     * @param peakCompressed Pointer to the peak-compressed audio buffer,
     *        or nullptr if peak-compressed audio should not be exported.
     * @param rmsCompressed Pointer to the RMS-compressed audio buffer,
     *        or nullptr if RMS-compressed audio should not be exported.
     * @param sampleRateForWav Sample rate used for WAV file export.
     * @param error Optional pointer for returning error messages.
     * @return True on success, false if an error occurred.
     */
    bool exportAll(const juce::File& inputFile,
        const juce::String& metricsText,
        const juce::AudioBuffer<float>* peakCompressed, // nullptr => skip
        const juce::AudioBuffer<float>* rmsCompressed,  // nullptr => skip
        double sampleRateForWav,                        // from loader
        juce::String* error = nullptr);

    /**
    * Exports only the metrics text file for a given input file.
    *
    * @param inputFile The original input audio file.
    * @param metricsText Textual representation of the computed metrics.
    * @param error Optional pointer for returning error messages.
    * @return True on success, false if an error occurred.
    */
    bool exportMetricsOnly(const juce::File& inputFile,
        const juce::String& metricsText,
        juce::String* error = nullptr);

    /**
     * Returns the output directory used for exporting results.
     *
     * @return The output folder.
     */
    juce::File getOutputFolder() const { return outputFolder; }

private:
    /**
     * Ensures that the output directory exists, creating it if necessary.
     *
     * @param error Optional pointer for returning error messages.
     * @return True if the directory exists or was created successfully.
     */
    bool ensureOutputFolder(juce::String* error);

    /**
     * Generates a unique output file name based on the input file name
     * and a specified suffix.
     *
     * @param inputFile The original input audio file.
     * @param suffix Descriptive suffix added to the file name.
     * @param extension File extension (e.g., ".wav", ".txt").
     * @return A file path that does not overwrite existing files.
     */
    juce::File makeUniqueFile(const juce::File& inputFile,
        const juce::String& suffix,
        const juce::String& extension) const;

    /**
    * Writes a text file to disk.
    *
    * @param file Target file path.
    * @param content Text content to write.
    * @param error Optional pointer for returning error messages.
    * @return True on success, false if an error occurred.
    */
    bool saveText(const juce::File& file, const juce::String& content, juce::String* error) const;

    /**
     * Writes an audio buffer to a WAV file.
     *
     * @param file Target file path.
     * @param buffer Audio buffer to be written.
     * @param sampleRate Sample rate used for the WAV file.
     * @param bitDepth Bit depth of the exported audio.
     * @param error Optional pointer for returning error messages.
     * @return True on success, false if an error occurred.
     */
    bool saveWav(const juce::File& file,
        const juce::AudioBuffer<float>& buffer,
        double sampleRate,
        int bitDepth,
        juce::String* error) const;

private:
    Config cfg;
    juce::File outputFolder;
};
