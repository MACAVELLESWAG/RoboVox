#include "PluginProcessor.h"
#include "PluginEditor.h"

RoboVoxAudioProcessor::RoboVoxAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Parameters are read fresh every block (avoids atomic copy issues)
}

RoboVoxAudioProcessor::~RoboVoxAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout RoboVoxAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "speed", "Speed (WPM)", juce::NormalisableRange<float> (80.0f, 450.0f, 1.0f), 175.0f,
        juce::AudioParameterFloatAttributes().withLabel ("WPM")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "pitch", "Pitch Offset", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "gain", "Gain", juce::NormalisableRange<float> (0.0f, 1.5f, 0.01f), 0.8f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "robotic", "Robotic Intensity", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f));

    return { params.begin(), params.end() };
}

void RoboVoxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    tts.prepareToPlay (sampleRate, samplesPerBlock);
}

void RoboVoxAudioProcessor::releaseResources()
{
    tts.releaseResources();
}

bool RoboVoxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void RoboVoxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // Read parameters fresh every block (safe, no atomic copy issues)
    if (auto* p = apvts.getRawParameterValue("speed"))   tts.setSpeed(*p);
    if (auto* p = apvts.getRawParameterValue("pitch"))   tts.setPitchOffsetSemitones(*p);
    if (auto* p = apvts.getRawParameterValue("gain"))    tts.setGain(*p);
    if (auto* p = apvts.getRawParameterValue("robotic")) tts.setRoboticIntensity(*p);

    // Handle MIDI (monophonic last-note priority)
    processMidi (midiMessages);

    // Clear buffer then let TTS fill it
    buffer.clear();
    tts.processBlock (buffer);
}

void RoboVoxAudioProcessor::processMidi (juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            tts.triggerNoteOn (msg.getNoteNumber(), msg.getFloatVelocity());
        }
        else if (msg.isNoteOff())
        {
            if (tts.getCurrentMidiNote() == msg.getNoteNumber())
                tts.triggerNoteOff();
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            tts.triggerNoteOff();
        }
    }
}

juce::AudioProcessorEditor* RoboVoxAudioProcessor::createEditor()
{
    return new RoboVoxAudioProcessorEditor (*this);
}

void RoboVoxAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RoboVoxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}