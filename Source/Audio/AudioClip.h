#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Model/Clip.h"
#include <memory>

class AudioClip : public juce::PositionableAudioSource
{
public:
    AudioClip();
    ~AudioClip() override;

    // Load audio data from a file result
    bool loadFromBuffer(std::unique_ptr<juce::AudioBuffer<float>> buffer,
                        double sampleRate, const Clip& clipData);

    // PositionableAudioSource implementation
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    juce::int64 getTotalLength() const override;
    void setNextReadPosition(juce::int64 newPosition) override;
    juce::int64 getNextReadPosition() const override;
    bool isLooping() const override { return looping; }
    void setLooping(bool shouldLoop) override { looping = shouldLoop; }

    // Clip properties
    void setGain(float newGain) { gain = newGain; }
    float getGain() const { return gain; }

    void setMuted(bool shouldMute) { muted = shouldMute; }
    bool isMuted() const { return muted; }

    // Timeline info
    double getStartTime() const { return clipData.startTime; }
    double getDuration() const { return clipData.duration; }
    double getEndTime() const { return clipData.getEndTime(); }

    const Clip& getClipData() const { return clipData; }
    void setClipData(const Clip& data) { clipData = data; }

    // Check if position (in samples) falls within this clip
    bool containsPosition(juce::int64 positionInSamples, double projectSampleRate) const;

    // Get position within the clip buffer given global position
    juce::int64 getLocalPosition(juce::int64 globalPosition, double projectSampleRate) const;

private:
    std::unique_ptr<juce::AudioBuffer<float>> audioBuffer;
    Clip clipData;

    double sourceSampleRate = 0.0;
    double playbackSampleRate = 0.0;

    juce::int64 readPosition = 0;
    float gain = 1.0f;
    bool muted = false;
    bool looping = false;

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClip)
};
