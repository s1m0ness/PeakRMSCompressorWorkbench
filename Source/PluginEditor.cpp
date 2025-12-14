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
    progressBar(progressValue)

{
    // BUTTONS AND SLIDERS
    //==============================================================================

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

    // Add and make visible buttons and configure onClick()
    addAndMakeVisible(powerButton);
    powerButton.setButtonText("Power");
    powerButton.onClick = [this]() { updateParameterState(); };

    addAndMakeVisible(muteButton);
    muteButton.setButtonText("Mute");

    addAndMakeVisible(rmsSwitchButton);
    rmsSwitchButton.setButtonText("Switch to RMS");
    rmsSwitchButton.onClick = [this]() { updateParameterState(); };
    
    // Add progress bar for tracking the metrics extraction process
    addAndMakeVisible(progressBar);
    progressBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colours::darkgrey); // Background color
    progressBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colours::green);   // Foreground color
    progressBar.setVisible(false);

    // EXTRACT METRICS AND PRESETS
    //==============================================================================

    // Add extract metrics button and configure onClick() for extracting metrics
    addAndMakeVisible(extractMetricsButton);
    extractMetricsButton.setButtonText("Extract Metrics");
    extractMetricsButton.onClick = [this]() { handleExtractMetrics(); };

    // Add preset combo box and configure onClick() for applying parameters
    addAndMakeVisible(presetComboBox);
    fillPresetComboBox();
    presetComboBox.onChange = [this]() { handlePresetChange(); };

    // BUTTONS AND SLIDERS ATTACHMENTS
    //==============================================================================

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
    
    //==============================================================================
    // Add metering
    addAndMakeVisible(meter);
    meter.setMode(Meter::Mode::GR);

    setSize (1000, 600);
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

// SET BOUNDS FOR GUI ELEMENTS
//==============================================================================
void PeakRMSCompressorWorkbenchAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // ProgressBar for metrics extraction
    auto progressBarHeight = 30;
    auto progressBarArea = area.removeFromTop(progressBarHeight).reduced(5);
    progressBarArea.setWidth(progressBarArea.getWidth() / 2); // Set width to half
    progressBar.setBounds(progressBarArea);

    // Buttons
    auto buttonWidth = 150;
    auto buttonHeight = 30;
    const int buttonSpacing = 10; // Vertical spacing between buttons
    const int verticalOffset = 30; // Move all buttons down by 30 pixels (space for progress bar)

    powerButton.setBounds(10, 10 + verticalOffset, buttonWidth, buttonHeight);
    muteButton.setBounds(10, powerButton.getBottom() + buttonSpacing, buttonWidth, buttonHeight);
    rmsSwitchButton.setBounds(10, muteButton.getBottom() + buttonSpacing, buttonWidth, buttonHeight);
    extractMetricsButton.setBounds(muteButton.getRight() + 20, 10 + verticalOffset, buttonWidth, buttonHeight);

    // ComboBox
    presetComboBox.setBounds(extractMetricsButton.getX(),
        extractMetricsButton.getBottom() + buttonSpacing,
        buttonWidth, buttonHeight);

    // Meter
    auto meterWidth = 500;
    auto meterHeight = 150;
    auto meterY = 10; 
    auto meterX = getWidth() - meterWidth - 20;

    meter.setBounds(meterX, meterY, meterWidth, meterHeight);

    // Two columns for sliders
    auto columnSpacing = 20;
    auto slidersArea = area;
    slidersArea.removeFromTop(130); // Move the sliders area down by 100 pixels

    auto leftColumn = slidersArea.removeFromLeft(slidersArea.getWidth() / 2 - columnSpacing);
    auto rightColumn = slidersArea;

    auto addSliderAndLabel = [](juce::Rectangle<int>& colArea,
        juce::Label& label, juce::Slider& slider, int labelWidth)
        {
            label.setBounds(colArea.removeFromTop(30).removeFromLeft(labelWidth));
            slider.setBounds(colArea.removeFromTop(40).reduced(5));
        };

    // Peak sliders in the left column
    auto labelWidth = 120;
    addSliderAndLabel(leftColumn, peakThresholdLabel, peakThresholdSlider, labelWidth);
    addSliderAndLabel(leftColumn, peakRatioLabel, peakRatioSlider, labelWidth);
    addSliderAndLabel(leftColumn, peakAttackLabel, peakAttackSlider, labelWidth);
    addSliderAndLabel(leftColumn, peakReleaseLabel, peakReleaseSlider, labelWidth);
    addSliderAndLabel(leftColumn, peakKneeLabel, peakKneeSlider, labelWidth);
    addSliderAndLabel(leftColumn, peakMakeupLabel, peakMakeupSlider, labelWidth);

    // RMS sliders in the right column
    addSliderAndLabel(rightColumn, rmsThresholdLabel, rmsThresholdSlider, labelWidth);
    addSliderAndLabel(rightColumn, rmsRatioLabel, rmsRatioSlider, labelWidth);
    addSliderAndLabel(rightColumn, rmsAttackLabel, rmsAttackSlider, labelWidth);
    addSliderAndLabel(rightColumn, rmsReleaseLabel, rmsReleaseSlider, labelWidth);
    addSliderAndLabel(rightColumn, rmsKneeLabel, rmsKneeSlider, labelWidth);
    addSliderAndLabel(rightColumn, rmsMakeupLabel, rmsMakeupSlider, labelWidth);
}

// ADDITIONAL FUNCTIONS
//==============================================================================

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

    progressValue = audioProcessor.getMetricsExtractionEngine().getProgress();
    progressBar.repaint();
}

void PeakRMSCompressorWorkbenchAudioProcessorEditor::updateParameterState()
{
    if (powerButton.getToggleState()) {
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

        // Update the compression mode when the power is reset
        audioProcessor.updateCompressionMode(isRMSMode);
    } else {
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

void PeakRMSCompressorWorkbenchAudioProcessorEditor::fillPresetComboBox()
{
    presetComboBox.clear(); // Clear existing items

    for (const auto& [presetId, preset] : audioProcessor.PresetParameters) {
        presetComboBox.addItem(preset.name, presetId); // Use name and id
    }
    presetComboBox.setSelectedId(0, juce::dontSendNotification); // Default to "None"
}

void PeakRMSCompressorWorkbenchAudioProcessorEditor::handleExtractMetrics()
{
    auto& metricsExtractionEngine = audioProcessor.getMetricsExtractionEngine();
    auto& fileLoader = audioProcessor.getAudioFileLoader();

    if (metricsExtractionEngine.isProcessing())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Processing",
            "Metrics extraction is already running.");
        return;
    }

    isMuted = muteButton.getToggleState();
    if (!isMuted)
    {
        muteButton.setToggleState(true, juce::dontSendNotification);
        audioProcessor.isMuted = true;
    }

    auto file = fileLoader.chooseAudioFile();

    if (!file.existsAsFile())
    {
        if (!isMuted)
        {
            muteButton.setToggleState(false, juce::dontSendNotification);
            audioProcessor.isMuted = false;
        }
        return;
    }

    // Lock UI
    powerButton.setToggleState(false, juce::dontSendNotification);
    powerButton.setEnabled(false);
    updateParameterState();

    audioProcessor.peakCompressor.setPower(true);
    audioProcessor.rmsCompressor.setPower(true);

    progressBar.setVisible(true);
    progressValue = 0.0;

    if (extractionThread.joinable())
        extractionThread.join();

    // Let the metrics extraction run in a separate thread
    extractionThread = std::thread([this, &metricsExtractionEngine, file]()
        {
            metricsExtractionEngine.run(file); // ENGINE decides validity

            juce::MessageManager::callAsync([this]()
                {
                    powerButton.setToggleState(true, juce::dontSendNotification);
                    powerButton.setEnabled(true);

                    if (!isMuted)
                    {
                        muteButton.setToggleState(false, juce::dontSendNotification);
                        audioProcessor.isMuted = false;
                    }

                    updateParameterState();

                    audioProcessor.peakCompressor.setPower(false);
                    audioProcessor.rmsCompressor.setPower(false);

                    progressBar.setVisible(false);

                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::InfoIcon,
                        "Done",
                        "Metrics extraction finished.");
                });
        });
    }


void PeakRMSCompressorWorkbenchAudioProcessorEditor::handlePresetChange() {
    int selectedPresetId = presetComboBox.getSelectedId();

    if (selectedPresetId > 0) {
        audioProcessor.applyPreset(selectedPresetId);

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
    }
}
