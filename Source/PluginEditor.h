#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RoboVoxAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit RoboVoxAudioProcessorEditor (RoboVoxAudioProcessor&);
    ~RoboVoxAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RoboVoxAudioProcessor& processorRef;

    // Controls
    juce::TextEditor textInput;
    juce::Slider speedSlider, pitchSlider, gainSlider, roboticSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttach, pitchAttach, gainAttach, roboticAttach;

    juce::Label titleLabel, statusLabel, hintLabel;
    juce::TextButton triggerButton;

    // Simple dark sci-fi LookAndFeel
    class DarkLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        DarkLookAndFeel();
    };

    DarkLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoboVoxAudioProcessorEditor)
};