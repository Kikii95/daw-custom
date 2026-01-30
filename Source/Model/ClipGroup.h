#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>
#include <utility>

/**
 * ClipGroup: Groups multiple clips across tracks for simultaneous editing.
 *
 * When clips are grouped:
 * - Selecting one selects all in the group
 * - Moving one moves all in the group
 * - Deleting one offers to delete all or just that one
 */
struct ClipGroup
{
    juce::Uuid id;
    juce::String name;

    // List of clips in this group: pairs of (trackId, clipId)
    std::vector<std::pair<juce::Uuid, juce::Uuid>> clips;

    // Group colour for visual identification
    juce::Colour colour { juce::Colours::orange };

    ClipGroup() : id(juce::Uuid()) {}

    explicit ClipGroup(const juce::String& groupName)
        : id(juce::Uuid()), name(groupName) {}

    // Check if a clip is in this group
    bool containsClip(const juce::Uuid& trackId, const juce::Uuid& clipId) const
    {
        for (const auto& [tid, cid] : clips)
        {
            if (tid == trackId && cid == clipId)
                return true;
        }
        return false;
    }

    // Add a clip to the group
    void addClip(const juce::Uuid& trackId, const juce::Uuid& clipId)
    {
        if (!containsClip(trackId, clipId))
            clips.emplace_back(trackId, clipId);
    }

    // Remove a clip from the group
    bool removeClip(const juce::Uuid& trackId, const juce::Uuid& clipId)
    {
        auto it = std::find_if(clips.begin(), clips.end(),
            [&](const auto& pair) {
                return pair.first == trackId && pair.second == clipId;
            });

        if (it != clips.end())
        {
            clips.erase(it);
            return true;
        }
        return false;
    }

    // Check if group is empty or has only one clip (should be dissolved)
    bool shouldDissolve() const
    {
        return clips.size() <= 1;
    }
};
