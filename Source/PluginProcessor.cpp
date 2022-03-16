/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VoiceSamplerAudioProcessor::VoiceSamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    juce::MidiInputCallback(),
    juce::MidiKeyboardStateListener()
    
#endif
{
    outputWaveform.clear();

    // Load audio files
    File const dir = File::getCurrentWorkingDirectory();
    File const audioFilesFolder = dir.getChildFile("VoiceAudioFiles");
    Array<File> audioFiles = audioFilesFolder.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*.wav");

    formatManager.registerBasicFormats();
    waveformData.resize(audioFiles.size());

    // Load audio files in buffers
    for (int idx = 0; idx < audioFiles.size(); idx++)
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFiles[idx]));

        if (reader != nullptr)
        {
            // Set size of buffer
            auto duration = (float)reader->lengthInSamples / reader->sampleRate;
            waveformData[idx].fileBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples); // issue here

            // Read audio file data into buffer
            reader->read(&waveformData[idx].fileBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

            // Get name from file name (Which should hopefully be the midi note!) and assign to structure
            juce::String midiNote = audioFiles[idx].getFileName().upToFirstOccurrenceOf(".", false, true);
            waveformData[idx].midiNote = midiNote;
        }
    }

    
}

VoiceSamplerAudioProcessor::~VoiceSamplerAudioProcessor()
{
}

//==============================================================================
const juce::String VoiceSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VoiceSamplerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VoiceSamplerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

void VoiceSamplerAudioProcessor::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
    keyboardState.processNextMidiEvent(message);
}

void VoiceSamplerAudioProcessor::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
}

void VoiceSamplerAudioProcessor::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/)
{
}

bool VoiceSamplerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VoiceSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VoiceSamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VoiceSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VoiceSamplerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VoiceSamplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void VoiceSamplerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

void VoiceSamplerAudioProcessor::getWaveform(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    for (int idx = 0; idx < waveformData.size(); idx++)
    {
        if (waveformData[idx].midiNote == (juce::String)midiNoteNumber)
        {
            outputWaveform = AudioSampleBuffer(1, waveformData[idx].fileBuffer.getNumSamples()); // Resize waveform output to match the sample we want to play
            outputWaveform = waveformData[idx].fileBuffer;

        }

    }
}

void VoiceSamplerAudioProcessor::clearWaveformBuffer()
{
    outputWaveform = AudioSampleBuffer(1, 1); // Size of output waveform is abitrary. This will change.
    outputWaveform.clear();

}


void VoiceSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void VoiceSamplerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VoiceSamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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



void VoiceSamplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Smoothing to be applying on NoteOn and NoteOff
    bool midiNoteDown = false;
    bool midiNoteUp = false;
    
    // Go through midi buffer and check for any midi messages
    for (const auto midiMessage : midiMessages)
    {
        auto message = midiMessage.getMessage();
        if (message.isNoteOn())
        {
            startSampleOffset = 0;
            //message = juce::MidiMessage::noteOn(message.getChannel(), message.getNoteNumber(), 1.0f);
            DBG("Note number " << message.getNoteNumber() << ": On");
            getWaveform(&keyboardState, message.getChannel(), message.getNoteNumber(), message.getVelocity()); // Get the waveform we need before filling buffer

            midiNoteDown = true;
        }
        else if (message.isNoteOff())
        {
            DBG("Note number " << message.getNoteNumber() << ": Off");
            

            midiNoteUp = true;
        }
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.


    int outputSamplesRemaining = buffer.getNumSamples(); // Keep track of how many samples left in the buffer that needs filling
    int outputWriteSamplePos = 0; // Sample index in output buffer to start writing into

    // Continously write waveform buffer into output buffer untill entire output buffer is filled
    while (outputSamplesRemaining > 0)
    {
        int numSamplesToFill = juce::jmin(outputSamplesRemaining, outputWaveform.getNumSamples() - startSampleOffset); // Calculate how many samples we need to fill for a single cycle

        // Write waveform buffer into output buffer. Cut it short if we've reached end of output buffer
        for (auto channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            buffer.copyFrom(channel, outputWriteSamplePos, outputWaveform, 0, startSampleOffset, numSamplesToFill); // Note: Waveform buffers should only have 1 channel
        }

        outputSamplesRemaining -= numSamplesToFill; // Keep track of how many samples left to fill in output buffer
        outputWriteSamplePos += numSamplesToFill; // Keep track of which sample of the output buffer we're on

        // If we've reached the end of the output buffer, then we need to keep track of where the waveform is cut off for the next buffer
        startSampleOffset += numSamplesToFill; 
        if (startSampleOffset == outputWaveform.getNumSamples())
            startSampleOffset = 0;
    }

    // Apply ramp if needed
    if (midiNoteUp || midiNoteDown)
    {
        if (midiNoteDown)
            buffer.applyGainRamp(0, 1000, 0, 1);
        else if (midiNoteUp)
        {
            buffer.applyGainRamp(buffer.getNumSamples() - 1000, 1000, 1, 0); // Apply ramp to current buffer before clearing it
            clearWaveformBuffer();
            startSampleOffset = 0; // Since waveform buffer changes size, we need to reset the startSampleOffset to 0, so that we don't go into a negative start sample.
        }
    }

}

//==============================================================================
bool VoiceSamplerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VoiceSamplerAudioProcessor::createEditor()
{
    return new VoiceSamplerAudioProcessorEditor (*this);
}

//==============================================================================
void VoiceSamplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VoiceSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoiceSamplerAudioProcessor();

}
