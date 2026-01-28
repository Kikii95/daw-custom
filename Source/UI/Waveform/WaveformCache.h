#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>
#include <unordered_map>

class WaveformCache
{
public:
    WaveformCache(int maxCacheSize = 32);
    ~WaveformCache();

    // Get or create thumbnail for a file
    juce::AudioThumbnail* getThumbnail(const juce::File& file,
                                        juce::AudioFormatManager& formatManager);

    // Check if a file is cached
    bool isCached(const juce::File& file) const;

    // Clear cache
    void clear();

    // Set number of samples to use for thumbnail
    void setResolution(int samplesPerThumbSample);

private:
    std::unique_ptr<juce::AudioThumbnailCache> thumbnailCache;

    struct ThumbnailEntry
    {
        std::unique_ptr<juce::AudioThumbnail> thumbnail;
        juce::int64 lastAccess;
    };

    std::unordered_map<juce::String, ThumbnailEntry> thumbnails;
    int resolution = 512;

    juce::CriticalSection cacheLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformCache)
};
