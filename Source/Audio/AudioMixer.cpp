#include "AudioMixer.h"

AudioMixer::AudioMixer()
{
}

AudioMixer::~AudioMixer()
{
    releaseResources();
}

void AudioMixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;

    juce::ScopedLock sl(trackLock);
    for (auto& track : tracks)
        track->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioMixer::releaseResources()
{
    juce::ScopedLock sl(trackLock);
    for (auto& track : tracks)
        track->releaseResources();
}

void AudioMixer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Clear output first
    bufferToFill.clearActiveBufferRegion();

    // Check if playing (via transport)
    if (transport != nullptr && !transport->isPlaying())
        return;

    // Check for solo tracks
    bool hasSolo = hasAnySoloTracks();

    // Mix all tracks
    {
        juce::ScopedLock sl(trackLock);

        for (auto& track : tracks)
        {
            // Skip muted tracks
            if (track->isMuted())
                continue;

            // If any track is soloed, only play soloed tracks
            if (hasSolo && !track->isSolo())
                continue;

            track->getNextAudioBlock(bufferToFill);
        }
    }

    // Apply master volume
    float master = masterVolume.load();
    bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples, master);

    // Update peak meters
    auto* buffer = bufferToFill.buffer;
    if (buffer->getNumChannels() >= 2)
    {
        float left = buffer->getMagnitude(0, bufferToFill.startSample, bufferToFill.numSamples);
        float right = buffer->getMagnitude(1, bufferToFill.startSample, bufferToFill.numSamples);

        // Update peaks with decay
        float currentLeft = leftPeak.load();
        float currentRight = rightPeak.load();

        leftPeak.store(juce::jmax(left, currentLeft * 0.95f));
        rightPeak.store(juce::jmax(right, currentRight * 0.95f));

        // Send to visualizer callback
        if (audioOutputCallback)
        {
            audioOutputCallback(buffer->getReadPointer(0, bufferToFill.startSample),
                                buffer->getReadPointer(1, bufferToFill.startSample),
                                bufferToFill.numSamples);
        }
    }

    // Advance transport position
    if (transport != nullptr)
        transport->advancePosition(bufferToFill.numSamples, currentSampleRate);
}

AudioTrack* AudioMixer::addTrack()
{
    juce::ScopedLock sl(trackLock);

    auto track = std::make_unique<AudioTrack>();

    if (currentSampleRate > 0)
        track->prepareToPlay(currentBlockSize, currentSampleRate);

    tracks.push_back(std::move(track));
    return tracks.back().get();
}

void AudioMixer::removeTrack(const juce::Uuid& trackId)
{
    juce::ScopedLock sl(trackLock);

    tracks.erase(
        std::remove_if(tracks.begin(), tracks.end(),
            [&trackId](const std::unique_ptr<AudioTrack>& track)
            {
                return track->getId() == trackId;
            }),
        tracks.end());
}

void AudioMixer::clearTracks()
{
    juce::ScopedLock sl(trackLock);
    tracks.clear();
}

AudioTrack* AudioMixer::getTrack(int index)
{
    juce::ScopedLock sl(trackLock);

    if (index >= 0 && index < static_cast<int>(tracks.size()))
        return tracks[static_cast<size_t>(index)].get();

    return nullptr;
}

AudioTrack* AudioMixer::getTrackById(const juce::Uuid& trackId)
{
    juce::ScopedLock sl(trackLock);

    for (auto& track : tracks)
    {
        if (track->getId() == trackId)
            return track.get();
    }

    return nullptr;
}

void AudioMixer::moveTrack(int fromIndex, int toIndex)
{
    juce::ScopedLock sl(trackLock);

    auto numTracks = static_cast<int>(tracks.size());
    if (fromIndex < 0 || fromIndex >= numTracks ||
        toIndex < 0 || toIndex >= numTracks ||
        fromIndex == toIndex)
        return;

    auto track = std::move(tracks[static_cast<size_t>(fromIndex)]);
    tracks.erase(tracks.begin() + fromIndex);

    if (toIndex > fromIndex)
        --toIndex;

    tracks.insert(tracks.begin() + toIndex, std::move(track));
}

void AudioMixer::setTransportController(TransportController* controller)
{
    transport = controller;
}

void AudioMixer::setPosition(double timeInSeconds)
{
    if (currentSampleRate <= 0)
        return;

    juce::int64 positionInSamples = static_cast<juce::int64>(timeInSeconds * currentSampleRate);

    juce::ScopedLock sl(trackLock);
    for (auto& track : tracks)
        track->setPosition(positionInSamples);
}

double AudioMixer::getPosition() const
{
    if (transport != nullptr)
        return transport->getPosition();

    return 0.0;
}

double AudioMixer::getTotalDuration() const
{
    double maxDuration = 0.0;

    juce::ScopedLock sl(trackLock);
    for (const auto& track : tracks)
    {
        for (int i = 0; i < track->getNumClips(); ++i)
        {
            if (auto* clip = const_cast<AudioTrack*>(track.get())->getClip(i))
                maxDuration = juce::jmax(maxDuration, clip->getEndTime());
        }
    }

    return maxDuration;
}

bool AudioMixer::hasAnySoloTracks() const
{
    juce::ScopedLock sl(trackLock);

    for (const auto& track : tracks)
    {
        if (track->isSolo())
            return true;
    }

    return false;
}

void AudioMixer::resetPeaks()
{
    leftPeak.store(0.0f);
    rightPeak.store(0.0f);
}
