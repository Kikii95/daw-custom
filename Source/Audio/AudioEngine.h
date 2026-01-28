#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

class AudioEngine : public juce::AudioSource
{
public:
    AudioEngine();
    ~AudioEngine() override;

    void initialise(int numInputChannels = 0, int numOutputChannels = 2);
    void shutdown();

    // AudioSource interface
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    // Accessors
    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }
    double getSampleRate() const { return currentSampleRate; }
    int getBlockSize() const { return currentBlockSize; }
    bool isInitialised() const { return initialised; }

    // Audio source management
    void setSource(juce::AudioSource* newSource);
    void clearSource();

private:
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer sourcePlayer;

    juce::AudioSource* currentSource = nullptr;

    double currentSampleRate = 0.0;
    int currentBlockSize = 0;
    bool initialised = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
