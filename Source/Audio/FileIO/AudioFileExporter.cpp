#include "AudioFileExporter.h"

AudioFileExporter::AudioFileExporter()
{
    formatManager.registerBasicFormats();
}

bool AudioFileExporter::exportToFile(const juce::File& outputFile,
                                      const juce::AudioBuffer<float>& buffer,
                                      const ExportSettings& settings)
{
    auto writer = createWriter(outputFile, settings);

    if (writer == nullptr)
        return false;

    bool success = writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());

    if (!success)
        lastError = "Failed to write audio data";

    return success;
}

bool AudioFileExporter::exportFromSource(const juce::File& outputFile,
                                          juce::AudioSource& source,
                                          double duration,
                                          const ExportSettings& settings,
                                          std::function<void(double)> progressCallback)
{
    auto writer = createWriter(outputFile, settings);

    if (writer == nullptr)
        return false;

    // Calculate total samples
    juce::int64 totalSamples = static_cast<juce::int64>(duration * settings.sampleRate);

    // Prepare source
    source.prepareToPlay(512, settings.sampleRate);

    // Buffer for rendering
    juce::AudioBuffer<float> buffer(settings.numChannels, 512);
    juce::AudioSourceChannelInfo channelInfo(&buffer, 0, 512);

    juce::int64 samplesWritten = 0;

    while (samplesWritten < totalSamples)
    {
        int samplesToWrite = static_cast<int>(juce::jmin(
            juce::int64(512),
            totalSamples - samplesWritten
        ));

        channelInfo.numSamples = samplesToWrite;
        buffer.clear();
        source.getNextAudioBlock(channelInfo);

        if (!writer->writeFromAudioSampleBuffer(buffer, 0, samplesToWrite))
        {
            lastError = "Failed to write audio data at sample " + juce::String(samplesWritten);
            source.releaseResources();
            return false;
        }

        samplesWritten += samplesToWrite;

        // Progress callback
        if (progressCallback)
        {
            double progress = static_cast<double>(samplesWritten) / static_cast<double>(totalSamples);
            progressCallback(progress);
        }
    }

    source.releaseResources();

    DBG("Exported " + juce::String(samplesWritten) + " samples to " + outputFile.getFileName());

    return true;
}

juce::StringArray AudioFileExporter::getSupportedFormats() const
{
    return { "WAV", "FLAC", "OGG" };
}

std::unique_ptr<juce::AudioFormatWriter> AudioFileExporter::createWriter(
    const juce::File& file,
    const ExportSettings& settings)
{
    // Delete existing file
    if (file.existsAsFile())
        file.deleteFile();

    // Get appropriate format
    juce::AudioFormat* format = nullptr;

    switch (settings.format)
    {
        case Format::WAV:
            format = formatManager.findFormatForFileExtension("wav");
            break;
        case Format::FLAC:
            format = formatManager.findFormatForFileExtension("flac");
            break;
        case Format::OGG:
            format = formatManager.findFormatForFileExtension("ogg");
            break;
    }

    if (format == nullptr)
    {
        lastError = "Unsupported audio format";
        return nullptr;
    }

    // Create output stream
    auto outputStream = file.createOutputStream();

    if (outputStream == nullptr)
    {
        lastError = "Could not create output file: " + file.getFullPathName();
        return nullptr;
    }

    // Create writer
    juce::StringPairArray metadata;
    metadata.set("title", file.getFileNameWithoutExtension());

    std::unique_ptr<juce::AudioFormatWriter> writer;

    if (settings.format == Format::WAV)
    {
        writer.reset(format->createWriterFor(
            outputStream.release(),
            settings.sampleRate,
            static_cast<unsigned int>(settings.numChannels),
            settings.bitDepth,
            metadata,
            0
        ));
    }
    else
    {
        // FLAC/OGG with quality setting
        writer.reset(format->createWriterFor(
            outputStream.release(),
            settings.sampleRate,
            static_cast<unsigned int>(settings.numChannels),
            settings.bitDepth,
            metadata,
            settings.quality
        ));
    }

    if (writer == nullptr)
    {
        lastError = "Could not create audio writer";
    }

    return writer;
}
