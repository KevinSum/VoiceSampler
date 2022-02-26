#include "ReferenceCountedBuffer.h"


ReferenceCountedBuffer::ReferenceCountedBuffer(const juce::String& nameToUse, int numChannels, int numSamples)
    : name(nameToUse), buffer(numChannels, numSamples) // Initialize name and buffer 
{
    DBG(juce::String("Buffer named '") + name + "' constructed. numChannels = " + juce::String(numChannels) + ", numSamples = " + juce::String(numSamples));
}

ReferenceCountedBuffer::~ReferenceCountedBuffer()
{
    DBG(juce::String("Buffer named '") + name + "' destroyed");
}

juce::AudioSampleBuffer* ReferenceCountedBuffer::getAudioSampleBuffer()
{
    return &buffer;
}
