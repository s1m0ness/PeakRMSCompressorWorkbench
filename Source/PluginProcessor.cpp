/*
 * Implementation based on the JUCE framework.
 * This file contains the core audio processing logic for the plugin.
 *
 * Key Features:
 * - Handles audio processing, including compression.
 * - Implements RMS and Peak-based dynamic range compression using the JUCE DSP framework.
 * - Provides support for real-time and offline audio analysis.
 * - Saves compressed audio outputs and extracted metrics to user-defined locations.
 *
 * Additional Features:
 * - Extends the basic JUCE framework to include metrics extraction functionality, enabling analysis
 *   of dynamic range compression applied on audio signal, using peak-based or RMS-based level detection.
 *
 * NOTE: This implementation integrates with the JUCE AudioProcessor API and follows its standard plugin structure.
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
PeakRMSCompressorWorkbenchAudioProcessor::PeakRMSCompressorWorkbenchAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
    audioFileLoader(formatManager),
    dataExport(DataExport::Config{
        juce::File(Config::OutputPath::path),
        "PeakRMSCompressorWorkbench_testing_results",
        24,
        Config::saveCompressedFiles::save
        }),
    metricsExtractionEngine(
        audioFileLoader,
        dataExport,
        peakCompressor,
        rmsCompressor,
        metrics,
        parameters,
        MetricsExtractionEngine::Config{ 1024 }
    )
#endif
{
    parameters.addParameterListener("power", this);
    parameters.addParameterListener("mute", this);
    parameters.addParameterListener("isRMS", this);

    parameters.addParameterListener("peak_threshold", this);
    parameters.addParameterListener("peak_ratio", this);
    parameters.addParameterListener("peak_knee", this);
    parameters.addParameterListener("peak_attack", this);
    parameters.addParameterListener("peak_release", this);
    parameters.addParameterListener("peak_makeup", this);

    parameters.addParameterListener("rms_threshold", this);
    parameters.addParameterListener("rms_ratio", this);
    parameters.addParameterListener("rms_knee", this);
    parameters.addParameterListener("rms_attack", this);
    parameters.addParameterListener("rms_release", this);
    parameters.addParameterListener("rms_makeup", this);

    gainReduction = 0.0f;
    currentInput = -std::numeric_limits<float>::infinity();
    currentOutput = -std::numeric_limits<float>::infinity();

    formatManager.registerBasicFormats();
}

PeakRMSCompressorWorkbenchAudioProcessor::~PeakRMSCompressorWorkbenchAudioProcessor()
{
}

//==============================================================================
const juce::String PeakRMSCompressorWorkbenchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PeakRMSCompressorWorkbenchAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PeakRMSCompressorWorkbenchAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PeakRMSCompressorWorkbenchAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PeakRMSCompressorWorkbenchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PeakRMSCompressorWorkbenchAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PeakRMSCompressorWorkbenchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PeakRMSCompressorWorkbenchAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PeakRMSCompressorWorkbenchAudioProcessor::getProgramName(int index)
{
    return {};
}

void PeakRMSCompressorWorkbenchAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

// PREPARE DSP AND METRICS MODULES
//==============================================================================
void PeakRMSCompressorWorkbenchAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    peakCompressor.prepare({ sampleRate, static_cast<uint32>(samplesPerBlock), 2 });
    rmsCompressor.prepare({ sampleRate, static_cast<uint32>(samplesPerBlock), 2 });

    inLevelFollower.prepare(sampleRate);
    outLevelFollower.prepare(sampleRate);
    inLevelFollower.setPeakDecay(0.3f);
    outLevelFollower.setPeakDecay(0.3f);

    PresetParameters = createPresetParameters();
}

void PeakRMSCompressorWorkbenchAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PeakRMSCompressorWorkbenchAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

// REAL-TIME COMPRESSION OF INCOMING AUDIO
//==============================================================================
void PeakRMSCompressorWorkbenchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();

    // Compressor design supports only two-channel (stereo) audio format
    jassert(totalNumInputChannels == 2);
    jassert(totalNumOutputChannels == 2);

    // Clear input buffer
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update input peak metering
    inLevelFollower.updatePeak(buffer.getArrayOfReadPointers(), totalNumInputChannels, numSamples);
    currentInput = Decibels::gainToDecibels(inLevelFollower.getPeak());

    if (!isRMSMode) {
        // Apply peak compression
        peakCompressor.process(buffer, isRMSMode);
        // Get max. gain reduction for peak value for gain reduction metering
        gainReduction = peakCompressor.getMaxGainReduction();
    }
    else {
        // Apply rms compression
        rmsCompressor.process(buffer, isRMSMode);
        // Get max. gain reduction value for rms for gain reduction metering
        gainReduction = rmsCompressor.getMaxGainReduction();
    }

    // Update output peak metering
    outLevelFollower.updatePeak(buffer.getArrayOfReadPointers(), totalNumInputChannels, numSamples);
    currentOutput = Decibels::gainToDecibels(outLevelFollower.getPeak());

    if (isMuted) {
        buffer.clear(); // Silence the processed audio
        return;

    }
}

//==============================================================================
bool PeakRMSCompressorWorkbenchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PeakRMSCompressorWorkbenchAudioProcessor::createEditor()
{
    return new PeakRMSCompressorWorkbenchAudioProcessorEditor(*this, parameters);
    /*return new juce::GenericAudioProcessorEditor(*this);*/
}

//==============================================================================
void PeakRMSCompressorWorkbenchAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PeakRMSCompressorWorkbenchAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// PARAMETERS HANDLING
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PeakRMSCompressorWorkbenchAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterBool>("power", "Power", true));
    params.push_back(std::make_unique<AudioParameterBool>("mute", "Mute", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("isRMS", "Use RMS Detection", false));

    auto thresholdRange = NormalisableRange<float>(Constants::Parameter::thresholdStart,
        Constants::Parameter::thresholdEnd,
        Constants::Parameter::thresholdInterval);

    auto ratioRange = NormalisableRange<float>(Constants::Parameter::ratioStart,
        Constants::Parameter::ratioEnd,
        Constants::Parameter::ratioInterval);

    auto kneeRange = NormalisableRange<float>(Constants::Parameter::kneeStart,
        Constants::Parameter::kneeEnd,
        Constants::Parameter::kneeInterval);

    auto makeupRange = NormalisableRange<float>(Constants::Parameter::makeupStart,
        Constants::Parameter::makeupEnd,
        Constants::Parameter::makeupInterval);

    auto attackRange = NormalisableRange<float>(Constants::Parameter::attackStart,
        Constants::Parameter::attackEnd,
        Constants::Parameter::attackInterval);

    auto releaseRange = NormalisableRange<float>(Constants::Parameter::releaseStart,
        Constants::Parameter::releaseEnd,
        Constants::Parameter::releaseInterval);

    params.push_back(std::make_unique<AudioParameterFloat>("peak_threshold",
        "Peak Threshold",
        thresholdRange,
        0));

    params.push_back(std::make_unique<AudioParameterFloat>("peak_ratio",
        "Peak Ratio",
        ratioRange,
        3));

    params.push_back(std::make_unique<AudioParameterFloat>("peak_knee",
        "Peak Knee",
        kneeRange,
        0));

    params.push_back(std::make_unique<AudioParameterFloat>("peak_attack",
        "Peak Attack",
        attackRange,
        50));

    params.push_back(std::make_unique<AudioParameterFloat>("peak_release",
        "Peak Release",
        releaseRange,
        250));

    params.push_back(std::make_unique<AudioParameterFloat>("peak_makeup",
        "Peak Makeup",
        makeupRange,
        0));


    params.push_back(std::make_unique<AudioParameterFloat>("rms_threshold",
        "RMS Threshold",
        thresholdRange,
        0));

    params.push_back(std::make_unique<AudioParameterFloat>("rms_ratio",
        "RMS Ratio",
        ratioRange,
        3));

    params.push_back(std::make_unique<AudioParameterFloat>("rms_knee",
        "RMS Knee",
        kneeRange,
        0));

    params.push_back(std::make_unique<AudioParameterFloat>("rms_makeup",
        "RMS Makeup",
        makeupRange,
        0));


    params.push_back(std::make_unique<AudioParameterFloat>("rms_attack",
        "RMS Attack",
        attackRange,
        50));


    params.push_back(std::make_unique<AudioParameterFloat>("rms_release",
        "RMS Release",
        releaseRange,
        250));
    return { params.begin(), params.end() };
}

void PeakRMSCompressorWorkbenchAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "power") {
        peakCompressor.setPower(!static_cast<bool>(newValue));
        rmsCompressor.setPower(!static_cast<bool>(newValue));
    }
    else if (parameterID == "mute") isMuted = static_cast<bool>(newValue);
    else if (parameterID == "isRMS") isRMSMode = static_cast<bool>(newValue);

    // Peak parameters
    else if (parameterID == "peak_threshold") peakCompressor.setThreshold(newValue);
    else if (parameterID == "peak_ratio") peakCompressor.setRatio(newValue);
    else if (parameterID == "peak_knee") peakCompressor.setKnee(newValue);
    else if (parameterID == "peak_attack") peakCompressor.setAttack(newValue);
    else if (parameterID == "peak_release") peakCompressor.setRelease(newValue);
    else if (parameterID == "peak_makeup") peakCompressor.setMakeup(newValue);

    // RMS parameters
    else if (parameterID == "rms_threshold") rmsCompressor.setThreshold(newValue);
    else if (parameterID == "rms_ratio") rmsCompressor.setRatio(newValue);
    else if (parameterID == "rms_knee") rmsCompressor.setKnee(newValue);
    else if (parameterID == "rms_attack") rmsCompressor.setAttack(newValue);
    else if (parameterID == "rms_release") rmsCompressor.setRelease(newValue);
    else if (parameterID == "rms_makeup") rmsCompressor.setMakeup(newValue);
}

void PeakRMSCompressorWorkbenchAudioProcessor::updateCompressionMode(bool isRMSMode)
{
    if (isRMSMode) {
        rmsCompressor.setThreshold(*parameters.getRawParameterValue("rms_threshold"));
        rmsCompressor.setRatio(*parameters.getRawParameterValue("rms_ratio"));
        rmsCompressor.setAttack(*parameters.getRawParameterValue("rms_attack"));
        rmsCompressor.setRelease(*parameters.getRawParameterValue("rms_release"));
        rmsCompressor.setKnee(*parameters.getRawParameterValue("rms_knee"));
        rmsCompressor.setMakeup(*parameters.getRawParameterValue("rms_makeup"));
    }
    else {
        peakCompressor.setThreshold(*parameters.getRawParameterValue("peak_threshold"));
        peakCompressor.setRatio(*parameters.getRawParameterValue("peak_ratio"));
        peakCompressor.setAttack(*parameters.getRawParameterValue("peak_attack"));
        peakCompressor.setRelease(*parameters.getRawParameterValue("peak_release"));
        peakCompressor.setKnee(*parameters.getRawParameterValue("peak_knee"));
        peakCompressor.setMakeup(*parameters.getRawParameterValue("peak_makeup"));
    }
}

// LOADING AND APPLYING PRESETS
//==============================================================================

std::map<int, PresetStruct>  PeakRMSCompressorWorkbenchAudioProcessor::createPresetParameters() {
    std::map<int, PresetStruct> parameters;

    // Populate the parameters map dynamically
    for (const auto& preset : Preset::AllPresets) {
        parameters[preset.id] = {
            preset.name,
            {
                {"peak_threshold", preset.peak.threshold},
                {"peak_ratio", preset.peak.ratio},
                {"peak_attack", preset.peak.attack},
                {"peak_release", preset.peak.release},
                {"peak_knee", preset.peak.knee},
                {"peak_makeup", preset.peak.makeup},
                {"rms_threshold", preset.rms.threshold},
                {"rms_ratio", preset.rms.ratio},
                {"rms_attack", preset.rms.attack},
                {"rms_release", preset.rms.release},
                {"rms_knee", preset.rms.knee},
                {"rms_makeup", preset.rms.makeup}
            }
        };
    }
    return parameters;
}

void PeakRMSCompressorWorkbenchAudioProcessor::applyPreset(int presetId)
{
    // Check if the given presetId exists in the PresetParameters map
    auto presetIt = PresetParameters.find(presetId);
    if (presetIt != PresetParameters.end()) {
        const auto& preset = presetIt->second; // Access the Preset struct
        const auto& parametersMap = preset.parameters; // Get the parameters map for the preset

        // Lambda to set the parameters
        auto setParameter = [this](const juce::String& paramID, float value) {
            if (auto* parameter = parameters.getParameter(paramID)) {
                parameter->setValueNotifyingHost(parameters.getParameterRange(paramID).convertTo0to1(value));
            }
        };

        // Iterate over the parameters map and apply each parameter
        for (const auto& param : parametersMap) {
            setParameter(param.first, param.second);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PeakRMSCompressorWorkbenchAudioProcessor();
}