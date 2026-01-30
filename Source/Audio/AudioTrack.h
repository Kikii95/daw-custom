#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "AudioClip.h"
#include "DSP/EffectChain.h"
#include "Model/Track.h"
#include <vector>
#include <memory>

class AudioTrack : public juce::AudioSource
{
public:
    AudioTrack();
    ~AudioTrack() override;

    // AudioSource implementation
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    // Clip management
    void addClip(std::unique_ptr<AudioClip> clip);
    std::unique_ptr<AudioClip> removeClip(const juce::Uuid& clipId);  // Returns the removed clip
    void clearClips();

    int getNumClips() const { return static_cast<int>(clips.size()); }
    AudioClip* getClip(int index);
    AudioClip* getClip(const juce::Uuid& clipId);

    // Update clip timing (for trim operations)
    void updateClipTiming(const juce::Uuid& clipId, double newStartTime,
                          double newDuration, double newSourceOffset);

    // Update clip fade (for fade handle drags)
    void updateClipFade(const juce::Uuid& clipId, bool isFadeIn, double duration);

    // Update clip start time (for ripple edit)
    void updateClipStartTime(const juce::Uuid& clipId, double newStartTime);

    // Track properties
    void setVolume(float vol) { volume.store(juce::jmax(0.0f, vol)); }
    float getVolume() const { return volume.load(); }

    void setPan(float p) { pan.store(juce::jlimit(-1.0f, 1.0f, p)); }
    float getPan() const { return pan.load(); }

    void setMuted(bool m) { muted.store(m); }
    bool isMuted() const { return muted.load(); }

    void setSolo(bool s) { solo.store(s); }
    bool isSolo() const { return solo.load(); }

    // Position control (in samples)
    void setPosition(juce::int64 positionInSamples);
    juce::int64 getPosition() const { return position.load(); }

    // Track data reference
    void setTrackData(const Track& data) { trackData = data; }
    const Track& getTrackData() const { return trackData; }
    const juce::Uuid& getId() const { return trackData.id; }
    void setName(const juce::String& newName) { trackData.name = newName; }

    // Effect chain access
    EffectChain& getEffectChain() { return effectChain; }
    const EffectChain& getEffectChain() const { return effectChain; }

private:
    void applyPanning(juce::AudioBuffer<float>& buffer, float panValue);

    std::vector<std::unique_ptr<AudioClip>> clips;
    Track trackData;
    EffectChain effectChain;

    std::atomic<float> volume { 1.0f };
    std::atomic<float> pan { 0.0f };
    std::atomic<bool> muted { false };
    std::atomic<bool> solo { false };
    std::atomic<juce::int64> position { 0 };

    double currentSampleRate = 0.0;
    int currentBlockSize = 0;

    juce::AudioBuffer<float> tempBuffer;
    juce::CriticalSection clipLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioTrack)
};
