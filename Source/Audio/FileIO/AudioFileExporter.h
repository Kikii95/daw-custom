#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <functional>

class AudioFileExporter
{
public:
    enum class Format
    {
        WAV,
        FLAC,
        OGG
    };

    struct ExportSettings
    {
        Format format = Format::WAV;
        int sampleRate = 44100;
        int bitDepth = 16;      // For WAV
        int quality = 8;         // For FLAC/OGG (0-10)
        int numChannels = 2;
    };

    AudioFileExporter();
    ~AudioFileExporter() = default;

    // Export buffer to file
    bool exportToFile(const juce::File& outputFile,
                      const juce::AudioBuffer<float>& buffer,
                      const ExportSettings& settings);

    // Export from AudioSource (offline render)
    bool exportFromSource(const juce::File& outputFile,
                          juce::AudioSource& source,
                          double duration,
                          const ExportSettings& settings,
                          std::function<void(double)> progressCallback = nullptr);

    // Get supported formats
    juce::StringArray getSupportedFormats() const;

    // Get last error
    juce::String getLastError() const { return lastError; }

private:
    juce::AudioFormatManager formatManager;
    juce::String lastError;

    std::unique_ptr<juce::AudioFormatWriter> createWriter(const juce::File& file,
                                                           const ExportSettings& settings);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileExporter)
};
