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

    const auto totalLength = static_cast<double>(audioBuffer->getNumSamples());

    // Direction multiplier: +1 forward, -1 reverse
    const double direction = reversed ? -1.0 : 1.0;
    const double increment = playbackSpeed * direction;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Handle bounds (forward or reverse)
        if (readPosition >= totalLength - 1.0)
        {
            if (looping)
                readPosition = reversed ? totalLength - 1.0 : 0.0;
            else
            {
                for (int ch = 0; ch < outBuffer->getNumChannels(); ++ch)
                    outBuffer->clear(ch, startSample + sample, numSamples - sample);
                return;
            }
        }
        else if (readPosition < 0.0)
        {
            if (looping)
                readPosition = reversed ? totalLength - 1.0 : 0.0;
            else
            {
                for (int ch = 0; ch < outBuffer->getNumChannels(); ++ch)
                    outBuffer->clear(ch, startSample + sample, numSamples - sample);
                return;
            }
        }

        // Linear interpolation for variable speed playback
        const int idx0 = static_cast<int>(readPosition);
        const int idx1 = juce::jmin(idx0 + 1, static_cast<int>(totalLength) - 1);
        const float frac = static_cast<float>(readPosition - idx0);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s0 = audioBuffer->getSample(ch, idx0);
            float s1 = audioBuffer->getSample(ch, idx1);
            float interpolated = s0 + frac * (s1 - s0);  // Linear interpolation
            outBuffer->addSample(ch, startSample + sample, interpolated * gain);
        }

        // Handle mono to stereo
        if (numChannels == 1 && outBuffer->getNumChannels() > 1)
        {
            float s0 = audioBuffer->getSample(0, idx0);
            float s1 = audioBuffer->getSample(0, idx1);
            float interpolated = s0 + frac * (s1 - s0);
            for (int ch = 1; ch < outBuffer->getNumChannels(); ++ch)
                outBuffer->addSample(ch, startSample + sample, interpolated * gain);
        }

        readPosition += increment;
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
    readPosition = juce::jmax(0.0, static_cast<double>(newPosition));
}

juce::int64 AudioClip::getNextReadPosition() const
{
    juce::ScopedLock sl(lock);
    return static_cast<juce::int64>(readPosition);
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
