#include "WaveformCache.h"

WaveformCache::WaveformCache(int maxCacheSize)
{
    thumbnailCache = std::make_unique<juce::AudioThumbnailCache>(maxCacheSize);
}

WaveformCache::~WaveformCache()
{
    clear();
}

juce::AudioThumbnail* WaveformCache::getThumbnail(const juce::File& file,
                                                   juce::AudioFormatManager& formatManager)
{
    juce::ScopedLock sl(cacheLock);

    juce::String key = file.getFullPathName();

    // Check if already cached
    auto it = thumbnails.find(key);
    if (it != thumbnails.end())
    {
        it->second.lastAccess = juce::Time::currentTimeMillis();
        return it->second.thumbnail.get();
    }

    // Create new thumbnail
    auto thumbnail = std::make_unique<juce::AudioThumbnail>(
        resolution,
        formatManager,
        *thumbnailCache
    );

    // Load file
    if (file.existsAsFile())
    {
        thumbnail->setSource(new juce::FileInputSource(file));
    }

    auto* result = thumbnail.get();

    ThumbnailEntry entry;
    entry.thumbnail = std::move(thumbnail);
    entry.lastAccess = juce::Time::currentTimeMillis();

    thumbnails[key] = std::move(entry);

    return result;
}

bool WaveformCache::isCached(const juce::File& file) const
{
    juce::ScopedLock sl(cacheLock);
    return thumbnails.find(file.getFullPathName()) != thumbnails.end();
}

void WaveformCache::clear()
{
    juce::ScopedLock sl(cacheLock);
    thumbnails.clear();
}

void WaveformCache::setResolution(int samplesPerThumbSample)
{
    resolution = samplesPerThumbSample;
}
