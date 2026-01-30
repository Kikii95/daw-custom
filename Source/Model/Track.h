#pragma once

#include "Clip.h"
#include <juce_graphics/juce_graphics.h>
#include <vector>
#include <algorithm>

struct Track
{
    juce::Uuid id;
    juce::String name;

    // Track properties
    float volume = 1.0f;          // 0.0 - 2.0+
    float pan = 0.0f;             // -1.0 (left) to 1.0 (right)
    bool muted = false;
    bool solo = false;
    bool armed = false;           // For recording

    // Visual
    juce::Colour colour { juce::Colours::darkgrey };
    int height = 100;             // UI height in pixels

    // Clips on this track
    std::vector<Clip> clips;

    // Add clip
    void addClip(const Clip& clip)
    {
        clips.push_back(clip);
        sortClips();
    }

    // Remove clip by ID
    bool removeClip(const juce::Uuid& clipId)
    {
        auto it = std::find_if(clips.begin(), clips.end(),
            [&clipId](const Clip& c) { return c.id == clipId; });

        if (it != clips.end())
        {
            clips.erase(it);
            return true;
        }
        return false;
    }

    // Get clip by ID
    Clip* getClip(const juce::Uuid& clipId)
    {
        for (auto& clip : clips)
        {
            if (clip.id == clipId)
                return &clip;
        }
        return nullptr;
    }

    const Clip* getClip(const juce::Uuid& clipId) const
    {
        for (const auto& clip : clips)
        {
            if (clip.id == clipId)
                return &clip;
        }
        return nullptr;
    }

    // Get clip at time
    Clip* getClipAtTime(double time)
    {
        for (auto& clip : clips)
        {
            if (clip.containsTime(time))
                return &clip;
        }
        return nullptr;
    }

    const Clip* getClipAtTime(double time) const
    {
        for (const auto& clip : clips)
        {
            if (clip.containsTime(time))
                return &clip;
        }
        return nullptr;
    }

    // Get all clips that overlap a time range
    std::vector<Clip*> getClipsInRange(double start, double end)
    {
        std::vector<Clip*> result;
        for (auto& clip : clips)
        {
            if (clip.startTime < end && clip.getEndTime() > start)
                result.push_back(&clip);
        }
        return result;
    }

    // Get total duration (end of last clip)
    double getTotalDuration() const
    {
        double maxEnd = 0.0;
        for (const auto& clip : clips)
            maxEnd = std::max(maxEnd, clip.getEndTime());
        return maxEnd;
    }

private:
    void sortClips()
    {
        std::sort(clips.begin(), clips.end(),
            [](const Clip& a, const Clip& b) { return a.startTime < b.startTime; });
    }
};
