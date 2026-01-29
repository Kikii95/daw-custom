#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

class AudioBufferTests : public juce::UnitTest
{
public:
    AudioBufferTests() : UnitTest("AudioBuffer Tests") {}

    void runTest() override
    {
        beginTest("Buffer creation");
        {
            juce::AudioBuffer<float> buffer(2, 1024);
            expectEquals(buffer.getNumChannels(), 2);
            expectEquals(buffer.getNumSamples(), 1024);
        }

        beginTest("Buffer clear");
        {
            juce::AudioBuffer<float> buffer(2, 512);
            buffer.clear();

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* data = buffer.getReadPointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    expectEquals(data[i], 0.0f);
                }
            }
        }

        beginTest("Buffer gain application");
        {
            juce::AudioBuffer<float> buffer(1, 100);
            auto* data = buffer.getWritePointer(0);

            // Fill with 1.0
            for (int i = 0; i < 100; ++i)
                data[i] = 1.0f;

            // Apply 0.5 gain
            buffer.applyGain(0.5f);

            // Verify
            for (int i = 0; i < 100; ++i)
                expectWithinAbsoluteError(data[i], 0.5f, 0.0001f);
        }
    }
};

class SampleRateTests : public juce::UnitTest
{
public:
    SampleRateTests() : UnitTest("Sample Rate Tests") {}

    void runTest() override
    {
        beginTest("Common sample rates");
        {
            // Standard sample rates
            expect(44100.0 > 0);
            expect(48000.0 > 0);
            expect(96000.0 > 0);
        }

        beginTest("Time to samples conversion");
        {
            double sampleRate = 44100.0;
            double timeInSeconds = 1.0;
            int expectedSamples = 44100;

            int samples = static_cast<int>(timeInSeconds * sampleRate);
            expectEquals(samples, expectedSamples);
        }

        beginTest("Samples to time conversion");
        {
            double sampleRate = 48000.0;
            int samples = 48000;
            double expectedTime = 1.0;

            double time = samples / sampleRate;
            expectWithinAbsoluteError(time, expectedTime, 0.0001);
        }
    }
};

// Register tests
static AudioBufferTests audioBufferTests;
static SampleRateTests sampleRateTests;
