#include <JuceHeader.h>

class ReferenceCountedBuffer : public juce::ReferenceCountedObject
{
public:
	ReferenceCountedBuffer(const juce::String&, int, int);
	~ReferenceCountedBuffer();
	juce::AudioSampleBuffer* getAudioSampleBuffer();

	int position = 0;

	typedef juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> Ptr; 

private:
	juce::String name;
	juce::AudioSampleBuffer buffer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReferenceCountedBuffer)
};

