/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReferenceCountedBuffer.h"

//==============================================================================
/**
*/
class VoiceSamplerAudioProcessor  : 
    public juce::AudioProcessor,
    private juce::MidiInputCallback,
    private juce::MidiKeyboardStateListener
{
public:

    //==============================================================================
    VoiceSamplerAudioProcessor();
    ~VoiceSamplerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void getWaveform(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity);
    void clearWaveformBuffer();
private:
    // Waveform/buffer stuff
    struct WaveformData {
        juce::String midiNote;
        AudioSampleBuffer fileBuffer;
    };
    std::vector<WaveformData> waveformData;
    juce::AudioFormatManager formatManager;
    AudioSampleBuffer outputWaveform = AudioSampleBuffer(1, 1); // Size of output waveform is abitrary. This will change.
    int startSampleOffset = 0;


    // MIDI stuff
    bool isAddingFromMidiInput = false; // This flag is used to indicate that MIDI data is arriving from an external source, rather than mouse-clicks on the on-screen keyboard.
    juce::MidiKeyboardState keyboardState; // The MidiKeyboardState class keeps track of which MIDI keys are currently held down.

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceSamplerAudioProcessor)
};
