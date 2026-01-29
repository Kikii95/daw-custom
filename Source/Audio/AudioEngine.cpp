#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

void AudioEngine::initialise(int numInputChannels, int numOutputChannels)
{
    if (initialised)
        return;

    auto result = deviceManager.initialiseWithDefaultDevices(numInputChannels, numOutputChannels);

    if (result.isNotEmpty())
    {
        DBG("AudioEngine initialisation failed: " + result);
        return;
    }

    // FIX: Get sample rate BEFORE setting up audio callback
    // This ensures prepareToPlay() can be called correctly
    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        currentSampleRate = device->getCurrentSampleRate();
        currentBlockSize = device->getCurrentBufferSizeSamples();
    }

    // Now set up the audio chain with valid sample rate
    sourcePlayer.setSource(this);
    deviceManager.addAudioCallback(&sourcePlayer);

    initialised = true;
    DBG("AudioEngine initialised: " + juce::String(currentSampleRate) + " Hz, " +
        juce::String(currentBlockSize) + " samples");
}

void AudioEngine::shutdown()
{
    if (!initialised)
        return;

    deviceManager.removeAudioCallback(&sourcePlayer);
    sourcePlayer.setSource(nullptr);
    deviceManager.closeAudioDevice();

    initialised = false;
}

void AudioEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;

    if (currentSource != nullptr)
        currentSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioEngine::releaseResources()
{
    if (currentSource != nullptr)
        currentSource->releaseResources();
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (currentSource != nullptr)
    {
        currentSource->getNextAudioBlock(bufferToFill);
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
}

void AudioEngine::setSource(juce::AudioSource* newSource)
{
    currentSource = newSource;

    if (currentSource != nullptr)
    {
        // Fallback: get sample rate from device if not set yet
        if (currentSampleRate <= 0)
        {
            if (auto* device = deviceManager.getCurrentAudioDevice())
            {
                currentSampleRate = device->getCurrentSampleRate();
                currentBlockSize = device->getCurrentBufferSizeSamples();
            }
        }

        // Prepare source if we have valid audio settings
        if (currentSampleRate > 0)
            currentSource->prepareToPlay(currentBlockSize, currentSampleRate);
    }
}

void AudioEngine::clearSource()
{
    if (currentSource != nullptr)
    {
        currentSource->releaseResources();
        currentSource = nullptr;
    }
}
