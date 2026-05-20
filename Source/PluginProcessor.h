#pragma once

#include <JuceHeader.h>
#include "Core/TTSynthesizer.h"

class RoboVoxAudioProcessor : public juce::AudioProcessor
{
public:
    RoboVoxAudioProcessor();
    ~RoboVoxAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Public for editor access (APVTS)
    juce::AudioProcessorValueTreeState apvts;

    TTSynthesizer& getTTSynthesizer() { return tts; }

private:
    void processMidi (juce::MidiBuffer& midiMessages);

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    TTSynthesizer tts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoboVoxAudioProcessor)
};