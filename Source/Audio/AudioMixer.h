#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "AudioTrack.h"
#include "TransportController.h"
#include <vector>
#include <memory>
#include <functional>

class AudioMixer : public juce::AudioSource
{
public:
    AudioMixer();
    ~AudioMixer() override;

    // AudioSource implementation
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    // Track management
    AudioTrack* addTrack();
    void removeTrack(const juce::Uuid& trackId);
    void clearTracks();

    int getNumTracks() const { return static_cast<int>(tracks.size()); }
    AudioTrack* getTrack(int index);
    AudioTrack* getTrackById(const juce::Uuid& trackId);

    // Transport binding
    void setTransportController(TransportController* controller);

    // Master settings
    void setMasterVolume(float vol) { masterVolume.store(juce::jmax(0.0f, vol)); }
    float getMasterVolume() const { return masterVolume.load(); }

    // Position control
    void setPosition(double timeInSeconds);
    double getPosition() const;

    // Total duration (longest track)
    double getTotalDuration() const;

    // Solo handling
    bool hasAnySoloTracks() const;

    // Get peak levels for metering
    float getLeftPeak() const { return leftPeak.load(); }
    float getRightPeak() const { return rightPeak.load(); }
    void resetPeaks();

    // Audio output callback (for visualizers)
    using AudioOutputCallback = std::function<void(const float*, const float*, int)>;
    void setAudioOutputCallback(AudioOutputCallback callback) { audioOutputCallback = std::move(callback); }

private:
    std::vector<std::unique_ptr<AudioTrack>> tracks;

    TransportController* transport = nullptr;

    std::atomic<float> masterVolume { 1.0f };
    std::atomic<float> leftPeak { 0.0f };
    std::atomic<float> rightPeak { 0.0f };

    double currentSampleRate = 0.0;
    int currentBlockSize = 0;

    juce::CriticalSection trackLock;

    // Callback for visualizers
    AudioOutputCallback audioOutputCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMixer)
};
