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

        // Calculate fade gain based on position within clip
        double localPositionInSeconds = readPosition / sourceSampleRate;
        float fadeGain = calculateFadeGain(localPositionInSeconds);
        float totalGain = gain * fadeGain;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s0 = audioBuffer->getSample(ch, idx0);
            float s1 = audioBuffer->getSample(ch, idx1);
            float interpolated = s0 + frac * (s1 - s0);  // Linear interpolation
            outBuffer->addSample(ch, startSample + sample, interpolated * totalGain);
        }

        // Handle mono to stereo
        if (numChannels == 1 && outBuffer->getNumChannels() > 1)
        {
            float s0 = audioBuffer->getSample(0, idx0);
            float s1 = audioBuffer->getSample(0, idx1);
            float interpolated = s0 + frac * (s1 - s0);
            for (int ch = 1; ch < outBuffer->getNumChannels(); ++ch)
                outBuffer->addSample(ch, startSample + sample, interpolated * totalGain);
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

float AudioClip::applyFadeCurve(float t, FadeType type) const
{
    // Clamp t to [0, 1]
    t = juce::jlimit(0.0f, 1.0f, t);

    switch (type)
    {
        case FadeType::Linear:
            return t;

        case FadeType::Exponential:
            // Slow start, fast end (quadratic)
            return t * t;

        case FadeType::SCurve:
            // Smooth ease in/out (smoothstep)
            return t * t * (3.0f - 2.0f * t);

        case FadeType::Logarithmic:
            // Fast start, slow end (sqrt)
            return std::sqrt(t);

        default:
            return t;
    }
}

float AudioClip::calculateFadeGain(double localPositionInSeconds) const
{
    float fadeGain = 1.0f;

    const double fadeIn = clipData.fadeInDuration;
    const double fadeOut = clipData.fadeOutDuration;
    const double duration = clipData.duration;

    // Fade In (at the beginning of the clip)
    if (fadeIn > 0.0 && localPositionInSeconds < fadeIn)
    {
        float t = static_cast<float>(localPositionInSeconds / fadeIn);
        fadeGain *= applyFadeCurve(t, clipData.fadeInType);
    }

    // Fade Out (at the end of the clip)
    if (fadeOut > 0.0 && localPositionInSeconds > (duration - fadeOut))
    {
        float t = static_cast<float>((duration - localPositionInSeconds) / fadeOut);
        fadeGain *= applyFadeCurve(t, clipData.fadeOutType);
    }

    return fadeGain;
}
