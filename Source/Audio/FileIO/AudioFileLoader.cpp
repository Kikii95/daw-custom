#include "AudioFileLoader.h"

AudioFileLoader::AudioFileLoader()
{
    formatManager.registerBasicFormats();
}

AudioFileLoader::LoadResult AudioFileLoader::loadFile(const juce::File& file)
{
    LoadResult result;

    if (!file.existsAsFile())
    {
        result.error = "File does not exist: " + file.getFullPathName();
        return result;
    }

    auto reader = createReaderFor(file);

    if (reader == nullptr)
    {
        result.error = "Could not create reader for: " + file.getFullPathName();
        return result;
    }

    result.sampleRate = reader->sampleRate;
    result.numChannels = static_cast<int>(reader->numChannels);
    result.numSamples = reader->lengthInSamples;
    result.lengthInSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

    // Create buffer and read audio
    result.buffer = std::make_unique<juce::AudioBuffer<float>>(
        result.numChannels,
        static_cast<int>(result.numSamples)
    );

    if (!reader->read(result.buffer.get(), 0, static_cast<int>(result.numSamples), 0, true, true))
    {
        result.error = "Failed to read audio data from: " + file.getFullPathName();
        result.buffer.reset();
        return result;
    }

    DBG("Loaded audio file: " + file.getFileName() +
        " (" + juce::String(result.numChannels) + " ch, " +
        juce::String(result.sampleRate) + " Hz, " +
        juce::String(result.lengthInSeconds, 2) + " sec)");

    return result;
}

std::unique_ptr<juce::AudioFormatReader> AudioFileLoader::createReaderFor(const juce::File& file)
{
    return std::unique_ptr<juce::AudioFormatReader>(
        formatManager.createReaderFor(file)
    );
}

juce::StringArray AudioFileLoader::getSupportedExtensions() const
{
    juce::StringArray extensions;

    for (int i = 0; i < formatManager.getNumKnownFormats(); ++i)
    {
        auto* format = formatManager.getKnownFormat(i);
        extensions.addArray(format->getFileExtensions());
    }

    return extensions;
}

bool AudioFileLoader::isFormatSupported(const juce::File& file) const
{
    juce::String extension = file.getFileExtension().toLowerCase();

    for (int i = 0; i < formatManager.getNumKnownFormats(); ++i)
    {
        auto* format = formatManager.getKnownFormat(i);
        if (format->getFileExtensions().contains(extension))
            return true;
    }

    return false;
}
