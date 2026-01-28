#include "AudioClip.h"

AudioClip::AudioClip()
{
}

AudioClip::~AudioClip()
{
    releaseResources();
}

bool AudioClip::loadFromBuffer(std::unique_ptr<juce::AudioBuffer<float>> buffer,
                                double sampleRate, const Clip& clip)
{
    juce::ScopedLock sl(lock);

    if (buffer == nullptr || buffer->getNumSamples() == 0)
        return false;

    audioBuffer = std::move(buffer);
    sourceSampleRate = sampleRate;
    clipData = clip;
    readPosition = 0;

    return true;
}

void AudioClip::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    playbackSampleRate = sampleRate;
}

void AudioClip::releaseResources()
{
    // Keep the buffer, just reset playback state
    readPosition = 0;
}

void AudioClip::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::ScopedLock sl(lock);

    if (audioBuffer == nullptr || muted)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* outBuffer = bufferToFill.buffer;
    const int numSamples = bufferToFill.numSamples;
    const int startSample = bufferToFill.startSample;
    const int numChannels = juce::jmin(outBuffer->getNumChannels(), audioBuffer->getNumChannels());

    const auto totalLength = static_cast<juce::int64>(audioBuffer->getNumSamples());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (readPosition >= totalLength)
        {
            if (looping)
                readPosition = 0;
            else
            {
                // Clear remaining samples
                for (int ch = 0; ch < outBuffer->getNumChannels(); ++ch)
                    outBuffer->clear(ch, startSample + sample, numSamples - sample);
                return;
            }
        }

        const auto srcPos = static_cast<int>(readPosition);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sourceSample = audioBuffer->getSample(ch, srcPos);
            outBuffer->addSample(ch, startSample + sample, sourceSample * gain);
        }

        // Handle mono to stereo
        if (numChannels == 1 && outBuffer->getNumChannels() > 1)
        {
            float sourceSample = audioBuffer->getSample(0, srcPos);
            for (int ch = 1; ch < outBuffer->getNumChannels(); ++ch)
                outBuffer->addSample(ch, startSample + sample, sourceSample * gain);
        }

        ++readPosition;
    }
}

juce::int64 AudioClip::getTotalLength() const
{
    juce::ScopedLock sl(lock);
    return audioBuffer ? static_cast<juce::int64>(audioBuffer->getNumSamples()) : 0;
}

void AudioClip::setNextReadPosition(juce::int64 newPosition)
{
    juce::ScopedLock sl(lock);
    readPosition = juce::jmax(juce::int64(0), newPosition);
}

juce::int64 AudioClip::getNextReadPosition() const
{
    juce::ScopedLock sl(lock);
    return readPosition;
}

bool AudioClip::containsPosition(juce::int64 positionInSamples, double projectSampleRate) const
{
    double positionInSeconds = static_cast<double>(positionInSamples) / projectSampleRate;
    return clipData.containsTime(positionInSeconds);
}

juce::int64 AudioClip::getLocalPosition(juce::int64 globalPosition, double projectSampleRate) const
{
    double globalTime = static_cast<double>(globalPosition) / projectSampleRate;
    double localTime = clipData.getSourcePosition(globalTime);
    return static_cast<juce::int64>(localTime * sourceSampleRate);
}
