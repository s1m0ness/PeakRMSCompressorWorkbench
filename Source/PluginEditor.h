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

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "gui/include/Meter.h"
#include <../Source/util/include/Constants.h>


//==============================================================================
/**
*/
class PeakRMSCompressorWorkbenchAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    PeakRMSCompressorWorkbenchAudioProcessorEditor (PeakRMSCompressorWorkbenchAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~PeakRMSCompressorWorkbenchAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback();
    void updateParameterState();
    void populatePresetComboBox();
    void handleExtractMetrics();
    void handlePresetChange();

    PeakRMSCompressorWorkbenchAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::Slider peakThresholdSlider;
    juce::Slider peakRatioSlider;
    juce::Slider peakAttackSlider;
    juce::Slider peakReleaseSlider;
    juce::Slider peakKneeSlider;
    juce::Slider peakMakeupSlider;
    juce::Slider rmsThresholdSlider;
    juce::Slider rmsRatioSlider;
    juce::Slider rmsAttackSlider;
    juce::Slider rmsReleaseSlider;
    juce::Slider rmsKneeSlider;
    juce::Slider rmsMakeupSlider;

    juce::Label peakThresholdLabel, peakRatioLabel, peakAttackLabel, peakReleaseLabel, peakKneeLabel, peakMakeupLabel;
    juce::Label rmsThresholdLabel, rmsRatioLabel, rmsAttackLabel, rmsReleaseLabel, rmsKneeLabel, rmsMakeupLabel;
    juce::Label metricsLabel;
    juce::Label loadingLabel;
    
    juce::ToggleButton powerButton;
    juce::ToggleButton muteButton;
    juce::ToggleButton rmsSwitchButton;

    juce::TextButton extractMetricsButton;
    juce::ProgressBar progressBar;

    MeterBackground meterbg;
    Meter meter;

    juce::ComboBox presetComboBox;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> rmsSwitchButtonAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakKneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsKneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rmsMakeupAttachment;

    bool alreadyMuted; // to solve mute button on/off bugs if already toggled on during metrics extraction

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakRMSCompressorWorkbenchAudioProcessorEditor)
};
