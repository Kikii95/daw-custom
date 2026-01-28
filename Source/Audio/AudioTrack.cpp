#include "AudioTrack.h"

AudioTrack::AudioTrack()
{
}

AudioTrack::~AudioTrack()
{
    releaseResources();
}

void AudioTrack::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;

    tempBuffer.setSize(2, samplesPerBlockExpected);

    juce::ScopedLock sl(clipLock);
    for (auto& clip : clips)
        clip->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioTrack::releaseResources()
{
    juce::ScopedLock sl(clipLock);
    for (auto& clip : clips)
        clip->releaseResources();

    tempBuffer.setSize(0, 0);
}

void AudioTrack::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // If muted, output silence
    if (muted.load())
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* outBuffer = bufferToFill.buffer;
    const int numSamples = bufferToFill.numSamples;
    const int startSample = bufferToFill.startSample;

    // Clear temp buffer
    tempBuffer.clear();

    // Current position in project
    juce::int64 currentPos = position.load();
    double currentTime = static_cast<double>(currentPos) / currentSampleRate;

    // Mix all clips that are active at current position
    {
        juce::ScopedLock sl(clipLock);

        for (auto& clip : clips)
        {
            if (clip->isMuted())
                continue;

            // Check if clip is active at current position
            const auto& clipData = clip->getClipData();
            double clipStart = clipData.startTime;
            double clipEnd = clipData.getEndTime();

            // Time at end of this block
            double blockEndTime = currentTime + (static_cast<double>(numSamples) / currentSampleRate);

            // Skip if clip doesn't overlap this block
            if (clipEnd <= currentTime || clipStart >= blockEndTime)
                continue;

            // Calculate where in the clip to read from
            juce::int64 clipLocalPos = clip->getLocalPosition(currentPos, currentSampleRate);
            clip->setNextReadPosition(clipLocalPos);

            // Create a temp channel info for this clip
            juce::AudioSourceChannelInfo clipInfo(&tempBuffer, 0, numSamples);
            clip->getNextAudioBlock(clipInfo);
        }
    }

    // Apply volume
    float vol = volume.load();
    tempBuffer.applyGain(vol);

    // Apply panning
    float p = pan.load();
    if (std::abs(p) > 0.001f)
        applyPanning(tempBuffer, p);

    // Add to output buffer
    for (int ch = 0; ch < juce::jmin(outBuffer->getNumChannels(), tempBuffer.getNumChannels()); ++ch)
    {
        outBuffer->addFrom(ch, startSample, tempBuffer, ch, 0, numSamples);
    }

    // Advance position
    position.store(currentPos + numSamples);
}

void AudioTrack::addClip(std::unique_ptr<AudioClip> clip)
{
    if (clip == nullptr)
        return;

    juce::ScopedLock sl(clipLock);

    if (currentSampleRate > 0)
        clip->prepareToPlay(currentBlockSize, currentSampleRate);

    clips.push_back(std::move(clip));
}

void AudioTrack::removeClip(const juce::Uuid& clipId)
{
    juce::ScopedLock sl(clipLock);

    clips.erase(
        std::remove_if(clips.begin(), clips.end(),
            [&clipId](const std::unique_ptr<AudioClip>& clip)
            {
                return clip->getClipData().id == clipId;
            }),
        clips.end());
}

void AudioTrack::clearClips()
{
    juce::ScopedLock sl(clipLock);
    clips.clear();
}

AudioClip* AudioTrack::getClip(int index)
{
    juce::ScopedLock sl(clipLock);

    if (index >= 0 && index < static_cast<int>(clips.size()))
        return clips[static_cast<size_t>(index)].get();

    return nullptr;
}

void AudioTrack::setPosition(juce::int64 positionInSamples)
{
    position.store(positionInSamples);
}

void AudioTrack::applyPanning(juce::AudioBuffer<float>& buffer, float panValue)
{
    if (buffer.getNumChannels() < 2)
        return;

    // Constant power panning
    // panValue: -1.0 (left) to 1.0 (right)
    float leftGain = std::cos((panValue + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f);
    float rightGain = std::sin((panValue + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f);

    buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
    buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);
}
