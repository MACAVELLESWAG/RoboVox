#pragma once

#include <JuceHeader.h>
#include <espeak-ng/speak_lib.h>
#include <atomic>
#include <thread>
#include <vector>
#include <string>

class TTSynthesizer
{
public:
    TTSynthesizer();
    ~TTSynthesizer();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    // Called from message thread (UI / parameter changes)
    void setText (const juce::String& newText);
    void setSpeed (float wpm);                    // 80 - 450
    void setPitchOffsetSemitones (float semitones);
    void setGain (float newGain);
    void setRoboticIntensity (float intensity);   // 0.0 - 1.0 → affects pitch range
    void setVoice (const juce::String& voiceName);

    // Trigger from audio thread (safe)
    void triggerNoteOn (int midiNoteNumber, float velocity);
    void triggerNoteOff();

    // Audio thread — real-time safe
    void processBlock (juce::AudioBuffer<float>& buffer);

    bool isCurrentlySpeaking() const noexcept;
    int  getCurrentMidiNote() const noexcept;

private:
    // Worker thread for synthesis (never on audio thread)
    void synthesisWorker();
    void performSynthesis (int midiNoteNumber);

    // espeak callback — collects samples
    static int espeakCallback (short* wav, int numsamples, espeak_EVENT* events);

    // Pitch mapping (MIDI → espeak 0-100)
    int mapMidiNoteToEspeakPitch (int midiNote) const;

    // Resample from espeak rate (22050) to host rate
    void resampleBuffer (const std::vector<short>& input, juce::AudioBuffer<float>& output);

    // State
    double hostSampleRate = 44100.0;
    int    espeakRate     = 22050;

    juce::String currentText;
    juce::CriticalSection textCs;

    float currentSpeed = 175.0f;
    float pitchOffset  = 0.0f;
    float gain         = 0.8f;
    float roboticIntensity = 0.3f;
    juce::String currentVoice = "en";

    std::atomic<int>  currentMidiNote { -1 };
    std::atomic<bool> noteIsOn      { false };
    std::atomic<bool> shouldSynthesize { false };
    std::atomic<int>  pendingMidiNote  { -1 };

    // Playback buffer (pre-allocated, real-time safe read)
    static constexpr int maxPlaybackSamples = 44100 * 20; // 20 seconds safety
    juce::AudioBuffer<float> playbackBuffer { 1, maxPlaybackSamples };
    std::atomic<int> playbackLength   { 0 };
    std::atomic<int> playbackReadPos  { 0 };
    std::atomic<bool> newBufferReady  { false };

    // Worker (using std::thread for simplicity and reliability here)
    std::thread workerThread;
    std::atomic<bool> threadShouldExit { false };

    // Temp storage for synthesis (worker thread only)
    std::vector<short> rawSamples;
    juce::AudioBuffer<float> tempResampledBuffer { 1, maxPlaybackSamples };

    // espeak handle
    bool espeakInitialized = false;

    // For C callback bridging (single instance assumption for v1)
    static TTSynthesizer* activeInstance;
    static void setActiveInstance (TTSynthesizer* inst) { activeInstance = inst; }
};