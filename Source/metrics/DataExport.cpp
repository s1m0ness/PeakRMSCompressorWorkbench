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
#include "include/DataExport.h"

DataExport::DataExport(Config c) : cfg(std::move(c))
{
    if (cfg.baseDirectory == juce::File())
        cfg.baseDirectory = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
}

bool DataExport::exportAll(const juce::File& inputFile,
    const juce::String& metricsText,
    const juce::AudioBuffer<float>* peakCompressed,
    const juce::AudioBuffer<float>* rmsCompressed,
    double sampleRateForWav,
    juce::String* error)
{
    if (!ensureOutputFolder(error))
        return false;

    // Always export metrics text.
    if (!exportMetricsOnly(inputFile, metricsText, error))
        return false;

    if (!cfg.exportWavs)
        return true;

    // Export optional WAVs (if buffers provided).
    if (peakCompressed != nullptr)
    {
        auto peakFile = makeUniqueFile(inputFile, "Peak_compressed", ".wav");
        if (!saveWav(peakFile, *peakCompressed, sampleRateForWav, cfg.bitDepth, error))
            return false;
    }

    if (rmsCompressed != nullptr)
    {
        auto rmsFile = makeUniqueFile(inputFile, "RMS_compressed", ".wav");
        if (!saveWav(rmsFile, *rmsCompressed, sampleRateForWav, cfg.bitDepth, error))
            return false;
    }

    return true;
}

bool DataExport::exportMetricsOnly(const juce::File& inputFile,
    const juce::String& metricsText,
    juce::String* error)
{
    if (!ensureOutputFolder(error))
        return false;

    auto metricsFile = makeUniqueFile(inputFile, "Metrics", ".txt");
    return saveText(metricsFile, metricsText, error);
}

bool DataExport::ensureOutputFolder(juce::String* error)
{
    if (outputFolder != juce::File() && outputFolder.exists())
        return true;

    outputFolder = cfg.baseDirectory.getChildFile(cfg.folderName);

    if (outputFolder.exists())
        return true;

    if (!outputFolder.createDirectory())
    {
        if (error) *error = "Failed to create output folder: " + outputFolder.getFullPathName();
        return false;
    }

    return true;
}

juce::File DataExport::makeUniqueFile(const juce::File& inputFile,
    const juce::String& suffix,
    const juce::String& extension) const
{
    auto base = outputFolder.getChildFile(inputFile.getFileNameWithoutExtension()
        + "_" + suffix + extension);

    auto unique = base;
    int counter = 2;

    while (unique.existsAsFile())
    {
        auto name = base.getFileNameWithoutExtension() + "_" + juce::String(counter) + extension;
        unique = base.getParentDirectory().getChildFile(name);
        ++counter;
    }

    return unique;
}

bool DataExport::saveText(const juce::File& file,
    const juce::String& content,
    juce::String* error) const
{
    auto stream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream());
    if (!stream)
    {
        if (error) *error = "Failed to open text stream for: " + file.getFullPathName();
        return false;
    }

    *stream << content;
    stream->flush();
    return true;
}

bool DataExport::saveWav(const juce::File& file,
    const juce::AudioBuffer<float>& buffer,
    double sampleRate,
    int bitDepth,
    juce::String* error) const
{
    if (sampleRate <= 0.0)
    {
        if (error) *error = "Invalid sample rate for WAV export.";
        return false;
    }

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        format.createWriterFor(new juce::FileOutputStream(file),
            sampleRate,
            (unsigned int)buffer.getNumChannels(),
            bitDepth,
            {},
            0));

    if (!writer)
    {
        if (error) *error = "Failed to create WAV writer for: " + file.getFullPathName();
        return false;
    }

    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    return true;
}
