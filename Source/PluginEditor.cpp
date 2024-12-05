/*
 * Implementation based on the JUCE framework.
 * This file contains the GUI implementation for the audio plugin.
 *
 * Key Features:
 * - Provides a user interface for controlling the plugin's parameters.
 * - Implements real-time visualization and interaction with the audio processing features.
 * - Includes controls for RMS and Peak-based compression parameters.
 *
 * Additional Features:
 * - Integrates dynamic controls for metrics extraction, including progress indicators and alerts.
 * - Extends the JUCE framework to provide a seamless GUI experience for interacting with audio processing.
 *
 * NOTE: This implementation follows the JUCE AudioProcessorEditor API and adheres to its standard plugin structure.
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PeakRMSCompressorWorkbenchAudioProcessorEditor::PeakRMSCompressorWorkbenchAudioProcessorEditor(
    PeakRMSCompressorWorkbenchAudioProcessor& p, 
    juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p), valueTreeState(vts), 
    progressBar(p.progress)

{
    // Add and make visible sliders and labels
    addAndMakeVisible(peakThresholdSlider);
    addAndMakeVisible(peakThresholdLabel);
    peakThresholdLabel.setText("Peak Threshold", juce::dontSendNotification);

    addAndMakeVisible(peakRatioSlider);
    addAndMakeVisible(peakRatioLabel);
    peakRatioLabel.setText("Peak Ratio", juce::dontSendNotification);

    addAndMakeVisible(peakAttackSlider);
    addAndMakeVisible(peakAttackLabel);
    peakAttackLabel.setText("Peak Attack", juce::dontSendNotification);

    addAndMakeVisible(peakReleaseSlider);
    addAndMakeVisible(peakReleaseLabel);
    peakReleaseLabel.setText("Peak Release", juce::dontSendNotification);

    addAndMakeVisible(peakKneeSlider);
    addAndMakeVisible(peakKneeLabel);
    peakKneeLabel.setText("Peak Knee", juce::dontSendNotification);

    addAndMakeVisible(peakMakeupSlider);
    addAndMakeVisible(peakMakeupLabel);
    peakMakeupLabel.setText("Peak Makeup", juce::dontSendNotification);

    addAndMakeVisible(rmsThresholdSlider);
    addAndMakeVisible(rmsThresholdLabel);
    rmsThresholdLabel.setText("RMS Threshold", juce::dontSendNotification);

    addAndMakeVisible(rmsRatioSlider);
    addAndMakeVisible(rmsRatioLabel);
    rmsRatioLabel.setText("RMS Ratio", juce::dontSendNotification);

    addAndMakeVisible(rmsAttackSlider);
    addAndMakeVisible(rmsAttackLabel);
    rmsAttackLabel.setText("RMS Attack", juce::dontSendNotification);

    addAndMakeVisible(rmsReleaseSlider);
    addAndMakeVisible(rmsReleaseLabel);
    rmsReleaseLabel.setText("RMS Release", juce::dontSendNotification);

    addAndMakeVisible(rmsKneeSlider);
    addAndMakeVisible(rmsKneeLabel);
    rmsKneeLabel.setText("RMS Knee", juce::dontSendNotification);

    addAndMakeVisible(rmsMakeupSlider);
    addAndMakeVisible(rmsMakeupLabel);
    rmsMakeupLabel.setText("RMS Makeup", juce::dontSendNotification);

    // Add and make visible buttons, add onClick listeners
    addAndMakeVisible(powerButton);
    powerButton.setButtonText("Power");
    powerButton.onClick = [this]() { updateParameterState(); };

    addAndMakeVisible(muteButton);
    muteButton.setButtonText("Mute");

    addAndMakeVisible(rmsSwitchButton);
    rmsSwitchButton.setButtonText("Switch to RMS");
    rmsSwitchButton.onClick = [this]() { updateParameterState(); };
    
    // Add progress bar for tracking the metrics extraction
    addAndMakeVisible(progressBar);
    progressBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colours::darkgrey); // Background color
    progressBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colours::green);   // Foreground color
    progressBar.setVisible(false); // hide at first

    // EXTRACT METRICS AND PRESETS

    // Add extract metrics button and configure onClick() for extracting metrics
    addAndMakeVisible(extractMetricsButton);
    extractMetricsButton.setButtonText("Extract Metrics");
    extractMetricsButton.onClick = [this]() {
        if (audioProcessor.isProcessing) {
            DBG("Extraction already in progress...");
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Processing",
                "Metrics extraction is already in progress.");
            return;
        }

        muteButton.setToggleState(true, juce::dontSendNotification);
        audioProcessor.isMuted = true;

        // Call loadFile directly on the main thread
        audioProcessor.loadFile();
        if (!audioProcessor.fileExists) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Error",
                "No valid file selected. Please try again.");
            
            muteButton.setToggleState(false, juce::dontSendNotification);
            audioProcessor.isMuted = false;
            return; // Do not proceed further if file loading failed
        }

        // Lock the UI to indicate loading
        juce::MessageManager::callAsync([this]() {
            powerButton.setToggleState(false, juce::dontSendNotification);
            powerButton.setEnabled(false);
            updateParameterState();
            audioProcessor.compressor.setPower(true);
            progressBar.setVisible(true);
            });

        // Start metrics extraction in a background thread
        audioProcessor.processingThread = std::thread([this]() {
            try {
                audioProcessor.extractMetrics();
            }
            catch (...) {
                DBG("Exception during extraction.");
            }

            // Unlock the UI after completion
            juce::MessageManager::callAsync([this]() {
                powerButton.setToggleState(true, juce::dontSendNotification);
                powerButton.setEnabled(true);
                muteButton.setToggleState(false, juce::dontSendNotification);
                audioProcessor.isMuted = false;
                updateParameterState();
                audioProcessor.compressor.setPower(false);
                progressBar.setVisible(false);

                // Add the success popup window here
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Success",
                    "Metrics have been successfully extracted and saved to: " +
                    juce::String(Constants::OutputPath::outputPath));
                });
            });

        // Check if thread is valid before detaching
        if (audioProcessor.processingThread.joinable()) {
            audioProcessor.processingThread.detach();
        }
        };

    // Add preset combo box and configure onClick() for applying parameters
    addAndMakeVisible(presetComboBox);
    populatePresetComboBox();
    presetComboBox.onChange = [this]() {
        int selectedPresetId = presetComboBox.getSelectedId();

        if (selectedPresetId > 0) {
            audioProcessor.applyPreset(selectedPresetId);

            // Update the sliders using getter methods
            juce::MessageManager::callAsync([this]() {
                peakThresholdSlider.setValue(audioProcessor.getParameterValue("peak_threshold"));
                peakRatioSlider.setValue(audioProcessor.getParameterValue("peak_ratio"));
                peakAttackSlider.setValue(audioProcessor.getParameterValue("peak_attack"));
                peakReleaseSlider.setValue(audioProcessor.getParameterValue("peak_release"));
                peakKneeSlider.setValue(audioProcessor.getParameterValue("peak_knee"));
                peakMakeupSlider.setValue(audioProcessor.getParameterValue("peak_makeup"));
                rmsThresholdSlider.setValue(audioProcessor.getParameterValue("rms_threshold"));
                rmsRatioSlider.setValue(audioProcessor.getParameterValue("rms_ratio"));
                rmsAttackSlider.setValue(audioProcessor.getParameterValue("rms_attack"));
                rmsReleaseSlider.setValue(audioProcessor.getParameterValue("rms_release"));
                rmsKneeSlider.setValue(audioProcessor.getParameterValue("rms_knee"));
                rmsMakeupSlider.setValue(audioProcessor.getParameterValue("rms_makeup"));
                });

            DBG("Preset applied and sliders updated.");
        }
    };

    // Buttons attachment
    powerButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        valueTreeState, "power", powerButton);
    muteButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        valueTreeState, "mute", muteButton);
    rmsSwitchButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        valueTreeState, "isRMS", rmsSwitchButton);

    // Peak Sliders attachment
    peakThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "peak_threshold", peakThresholdSlider);
    peakRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "peak_ratio", peakRatioSlider);
    peakKneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "peak_knee", peakKneeSlider);
    peakAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "peak_attack", peakAttackSlider);
    peakReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "peak_release", peakReleaseSlider);
    peakMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "peak_makeup", peakMakeupSlider);

    // RMS Sliders attachment
    rmsThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "rms_threshold", rmsThresholdSlider);
    rmsRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "rms_ratio", rmsRatioSlider);
    rmsKneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "rms_knee", rmsKneeSlider);
    rmsAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "rms_attack", rmsAttackSlider);
    rmsReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "rms_release", rmsReleaseSlider);
    rmsMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "rms_makeup", rmsMakeupSlider);
    
    setSize (500, 1200);
    updateParameterState();
    startTimerHz(60);
}

PeakRMSCompressorWorkbenchAudioProcessorEditor::~PeakRMSCompressorWorkbenchAudioProcessorEditor()
{
}

//==============================================================================
void PeakRMSCompressorWorkbenchAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey); // Background color
}

void PeakRMSCompressorWorkbenchAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // ProgressBar for metrics extraction
    auto progressBarHeight = 30;
    auto progressBarArea = area.removeFromTop(progressBarHeight).reduced(5);
    progressBar.setBounds(progressBarArea);

    // Buttons
    auto buttonArea = area.removeFromTop(40);
    auto buttonHeight = 30;
    auto powerButtonWidth = 100;
    auto rmsButtonWidth = 120;
    auto extractButtonWidth = 140;
    auto buttonY = buttonArea.getY() + (buttonArea.getHeight() - buttonHeight) / 2;
    auto buttonX = buttonArea.getX();
    auto buttonSpacing = 50;

    powerButton.setBounds(buttonX, buttonY, powerButtonWidth, buttonHeight);
    muteButton.setBounds(buttonX, buttonY + 20, powerButtonWidth, buttonHeight);

    buttonX = powerButton.getRight() + buttonSpacing;
    rmsSwitchButton.setBounds(buttonX, buttonY, rmsButtonWidth, buttonHeight);

    buttonX = rmsSwitchButton.getRight() + buttonSpacing;
    extractMetricsButton.setBounds(buttonX, buttonY, extractButtonWidth, buttonHeight);

    // ComboBox for presets
    auto comboBoxHeight = 30;
    auto comboBoxWidth = 150;
    presetComboBox.setBounds(buttonX, buttonY + 40, comboBoxWidth, comboBoxHeight);

    // Sliders and labels
    auto labelWidth = area.getWidth() / 3;
    auto sliderWidth = area.getWidth() - labelWidth - 20;
    auto sliderHeight = 40;

    peakThresholdLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    peakThresholdSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    peakRatioLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    peakRatioSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    peakAttackLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    peakAttackSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    peakReleaseLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    peakReleaseSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    peakKneeLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    peakKneeSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    peakMakeupLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    peakMakeupSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));
    
    rmsThresholdLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    rmsThresholdSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    rmsRatioLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    rmsRatioSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    rmsAttackLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    rmsAttackSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    rmsReleaseLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    rmsReleaseSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    rmsKneeLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    rmsKneeSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));

    rmsMakeupLabel.setBounds(area.removeFromTop(sliderHeight).removeFromLeft(labelWidth));
    rmsMakeupSlider.setBounds(area.removeFromTop(sliderHeight).reduced(5));
    
    // Metering
    addAndMakeVisible(meter);
    meter.setMode(Meter::Mode::GR);
    juce::FlexBox meterBox;
    meterBox.flexWrap = juce::FlexBox::Wrap::noWrap;
    meterBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    meterBox.items.add(juce::FlexItem(meter).withFlex(1).withMargin(1));
    meterBox.performLayout(area.toFloat());
}

void PeakRMSCompressorWorkbenchAudioProcessorEditor::timerCallback()
{
    int m = meter.getMode();
    switch (m)
    {
    case Meter::Mode::IN:
        meter.update(audioProcessor.currentInput);
        break;
    case Meter::Mode::OUT:
        meter.update(audioProcessor.currentOutput);
        break;
    case Meter::Mode::GR:
        meter.update(audioProcessor.gainReduction);
        break;
    default:
        break;
    }

    progressBar.repaint();
}

void PeakRMSCompressorWorkbenchAudioProcessorEditor::updateParameterState()
{
    if (powerButton.getToggleState()) 
    {
        meter.setEnabled(true);
        meter.setGUIEnabled(true);

        extractMetricsButton.setEnabled(true);
        rmsSwitchButton.setEnabled(true);
        presetComboBox.setEnabled(true);

        const bool isRMSMode = rmsSwitchButton.getToggleState();

        peakThresholdSlider.setEnabled(!isRMSMode);
        peakRatioSlider.setEnabled(!isRMSMode);
        peakAttackSlider.setEnabled(!isRMSMode);
        peakReleaseSlider.setEnabled(!isRMSMode);
        peakKneeSlider.setEnabled(!isRMSMode);
        peakMakeupSlider.setEnabled(!isRMSMode);

        rmsThresholdSlider.setEnabled(isRMSMode);
        rmsRatioSlider.setEnabled(isRMSMode);
        rmsAttackSlider.setEnabled(isRMSMode);
        rmsReleaseSlider.setEnabled(isRMSMode);
        rmsKneeSlider.setEnabled(isRMSMode);
        rmsMakeupSlider.setEnabled(isRMSMode);

        // Immediately update the compression mode
        audioProcessor.updateCompressionMode(isRMSMode);
    }
    else
    {
        meter.setEnabled(false);
        meter.setGUIEnabled(false);

        extractMetricsButton.setEnabled(false);
        rmsSwitchButton.setEnabled(false);
        presetComboBox.setEnabled(false);

        peakThresholdSlider.setEnabled(false);
        peakRatioSlider.setEnabled(false);
        peakAttackSlider.setEnabled(false);
        peakReleaseSlider.setEnabled(false);
        peakKneeSlider.setEnabled(false);
        peakMakeupSlider.setEnabled(false);

        rmsThresholdSlider.setEnabled(false);
        rmsRatioSlider.setEnabled(false);
        rmsAttackSlider.setEnabled(false);
        rmsReleaseSlider.setEnabled(false);
        rmsKneeSlider.setEnabled(false);
        rmsMakeupSlider.setEnabled(false);
    }
}

void PeakRMSCompressorWorkbenchAudioProcessorEditor::populatePresetComboBox()
{
    presetComboBox.clear(); // Clear existing items

    for (const auto& [presetId, preset] : PresetParameters)
    {
        presetComboBox.addItem(preset.name, presetId); // Use name and id
    }

    presetComboBox.setSelectedId(0, juce::dontSendNotification); // Default to "None"
}
