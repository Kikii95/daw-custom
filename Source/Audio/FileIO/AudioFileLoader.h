#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>

class AudioFileLoader
{
public:
    struct LoadResult
    {
        std::unique_ptr<juce::AudioBuffer<float>> buffer;
        double sampleRate = 0.0;
        int numChannels = 0;
        juce::int64 numSamples = 0;
        double lengthInSeconds = 0.0;
        juce::String error;

        bool isValid() const { return buffer != nullptr && error.isEmpty(); }
    };

    AudioFileLoader();
    ~AudioFileLoader() = default;

    // Load entire file into memory
    LoadResult loadFile(const juce::File& file);

    // Get reader for streaming (caller owns the reader)
    std::unique_ptr<juce::AudioFormatReader> createReaderFor(const juce::File& file);

    // Supported formats
    juce::StringArray getSupportedExtensions() const;
    bool isFormatSupported(const juce::File& file) const;

    // Access format manager
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

private:
    juce::AudioFormatManager formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileLoader)
};
