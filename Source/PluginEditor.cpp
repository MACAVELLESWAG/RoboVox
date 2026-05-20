#include "PluginEditor.h"

RoboVoxAudioProcessorEditor::RoboVoxAudioProcessorEditor (RoboVoxAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (520, 420); // Compact yet comfortable
    setResizable (true, true);
    setResizeLimits (420, 340, 900, 700);

    // Title
    titleLabel.setText ("ROBOVOX", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (28.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00e5ff)); // Cyan
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // Large text input — the star of the show
    textInput.setMultiLine (true);
    textInput.setReturnKeyStartsNewLine (true);
    textInput.setTabKeyUsedAsCharacter (false);
    textInput.setFont (juce::Font (18.0f));
    textInput.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff1a1a2e));
    textInput.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    textInput.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff00e5ff).withAlpha (0.4f));
    textInput.setText ("Hello. I am RoboVox. Draw monophonic notes in your piano roll to make me speak.");
    textInput.onTextChange = [this]
    {
        processorRef.getTTSynthesizer().setText (textInput.getText());
    };
    addAndMakeVisible (textInput);

    // Status
    statusLabel.setText ("READY — Monophonic | Enter text then play MIDI", juce::dontSendNotification);
    statusLabel.setFont (juce::Font (13.0f));
    statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xffa0a0a0));
    addAndMakeVisible (statusLabel);

    // Knobs — logical grouping
    auto setupSlider = [this](juce::Slider& s, const juce::String& /*suffix*/)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff00e5ff));
        s.setColour (juce::Slider::thumbColourId, juce::Colour (0xff00e5ff));
        s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        addAndMakeVisible (s);
    };

    setupSlider (speedSlider, "WPM");
    setupSlider (pitchSlider, "st");
    setupSlider (gainSlider, "");
    setupSlider (roboticSlider, "");

    speedAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.apvts, "speed",   speedSlider);
    pitchAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.apvts, "pitch",   pitchSlider);
    gainAttach    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.apvts, "gain",    gainSlider);
    roboticAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.apvts, "robotic", roboticSlider);

    // Labels under knobs
    // (For brevity we rely on slider text boxes + tooltips)

    // Optional manual trigger (useful in standalone)
    triggerButton.setButtonText ("FORCE SPEAK");
    triggerButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff00e5ff).withAlpha (0.2f));
    triggerButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    triggerButton.onClick = [this]
    {
        // Force a trigger at middle C for testing
        processorRef.getTTSynthesizer().triggerNoteOn (60, 0.8f);
    };
    addAndMakeVisible (triggerButton);

    hintLabel.setText ("Draw monophonic MIDI notes • Text restarts on each new note • Keep phrases short for rhythm", juce::dontSendNotification);
    hintLabel.setFont (juce::Font (11.0f, juce::Font::italic));
    hintLabel.setColour (juce::Label::textColourId, juce::Colour (0xff707070));
    hintLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (hintLabel);

    // Start timer for status updates
    startTimerHz (10);

    // Sync initial text
    processorRef.getTTSynthesizer().setText (textInput.getText());
}

RoboVoxAudioProcessorEditor::~RoboVoxAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
    stopTimer();
}

void RoboVoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark futuristic background
    g.fillAll (juce::Colour (0xff0f0f1a));

    // Subtle grid / power suit lines
    g.setColour (juce::Colour (0xff00e5ff).withAlpha (0.06f));
    for (int x = 0; x < getWidth(); x += 24)
        g.drawVerticalLine (x, 0.0f, (float)getHeight());
    for (int y = 0; y < getHeight(); y += 24)
        g.drawHorizontalLine (y, 0.0f, (float)getWidth());

    // Accent line under title
    g.setColour (juce::Colour (0xff00e5ff).withAlpha (0.5f));
    g.drawRect (20, 52, getWidth() - 40, 1);
}

void RoboVoxAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);

    titleLabel.setBounds (bounds.removeFromTop (45));

    // Status
    statusLabel.setBounds (bounds.removeFromTop (22));

    // Text input takes most space
    textInput.setBounds (bounds.removeFromTop (160));

    bounds.removeFromTop (10);

    // Knobs row
    auto knobsArea = bounds.removeFromTop (110);
    const int knobW = 85;
    const int spacing = (knobsArea.getWidth() - 4 * knobW) / 5;

    int x = knobsArea.getX() + spacing;
    speedSlider.setBounds   (x, knobsArea.getY(), knobW, 95); x += knobW + spacing;
    pitchSlider.setBounds   (x, knobsArea.getY(), knobW, 95); x += knobW + spacing;
    gainSlider.setBounds    (x, knobsArea.getY(), knobW, 95); x += knobW + spacing;
    roboticSlider.setBounds (x, knobsArea.getY(), knobW, 95);

    bounds.removeFromTop (10);

    // Button + hint
    triggerButton.setBounds (bounds.removeFromTop (32).withSizeKeepingCentre (140, 28));
    hintLabel.setBounds (bounds.removeFromTop (30));
}

void RoboVoxAudioProcessorEditor::timerCallback()
{
    auto& tts = processorRef.getTTSynthesizer();

    juce::String status = tts.isCurrentlySpeaking() ? "SPEAKING" : "READY";
    if (tts.getCurrentMidiNote() >= 0)
        status += "  |  Note: " + juce::MidiMessage::getMidiNoteName (tts.getCurrentMidiNote(), true, true, 3);

    statusLabel.setText (status, juce::dontSendNotification);
}

// DarkLookAndFeel implementation
RoboVoxAudioProcessorEditor::DarkLookAndFeel::DarkLookAndFeel()
{
    setColour (juce::Slider::thumbColourId, juce::Colour (0xff00e5ff));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff3a3a5a));
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff00e5ff).withAlpha (0.7f));
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1f1f35));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff00e5ff));
}