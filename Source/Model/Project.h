#pragma once

#include "Track.h"
#include "Marker.h"
#include "ClipGroup.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <memory>

// Edit modes for timeline operations
enum class EditMode
{
    Normal,  // Standard editing: clips don't affect each other
    Ripple   // Ripple edit: deleting/cutting shifts following clips
};

class Project
{
public:
    Project() : id(juce::Uuid()) {}

    // Project info
    juce::Uuid getId() const { return id; }
    juce::String getName() const { return name; }
    void setName(const juce::String& newName) { name = newName; }

    // File management
    juce::File getProjectFile() const { return projectFile; }
    void setProjectFile(const juce::File& file) { projectFile = file; }
    bool hasBeenSaved() const { return projectFile.existsAsFile(); }

    // Project settings
    double getSampleRate() const { return sampleRate; }
    void setSampleRate(double rate) { sampleRate = rate; }

    double getTempo() const { return tempo; }
    void setTempo(double bpm) { tempo = juce::jmax(20.0, juce::jmin(999.0, bpm)); }

    int getTimeSignatureNumerator() const { return timeSignatureNum; }
    int getTimeSignatureDenominator() const { return timeSignatureDenom; }
    void setTimeSignature(int num, int denom)
    {
        timeSignatureNum = num;
        timeSignatureDenom = denom;
    }

    // Track management
    Track& addTrack(const juce::String& trackName = "New Track")
    {
        Track track;
        track.id = juce::Uuid();
        track.name = trackName.isEmpty()
            ? "Track " + juce::String(tracks.size() + 1)
            : trackName;

        tracks.push_back(track);
        return tracks.back();
    }

    bool removeTrack(const juce::Uuid& trackId)
    {
        auto it = std::find_if(tracks.begin(), tracks.end(),
            [&trackId](const Track& t) { return t.id == trackId; });

        if (it != tracks.end())
        {
            tracks.erase(it);
            return true;
        }
        return false;
    }

    Track* getTrack(const juce::Uuid& trackId)
    {
        for (auto& track : tracks)
        {
            if (track.id == trackId)
                return &track;
        }
        return nullptr;
    }

    Track* getTrackByIndex(int index)
    {
        if (index >= 0 && index < static_cast<int>(tracks.size()))
            return &tracks[static_cast<size_t>(index)];
        return nullptr;
    }

    int getNumTracks() const { return static_cast<int>(tracks.size()); }
    std::vector<Track>& getTracks() { return tracks; }
    const std::vector<Track>& getTracks() const { return tracks; }

    void moveTrack(int fromIndex, int toIndex)
    {
        auto numTracks = static_cast<int>(tracks.size());
        if (fromIndex < 0 || fromIndex >= numTracks ||
            toIndex < 0 || toIndex >= numTracks ||
            fromIndex == toIndex)
            return;

        Track track = std::move(tracks[static_cast<size_t>(fromIndex)]);
        tracks.erase(tracks.begin() + fromIndex);

        if (toIndex > fromIndex)
            --toIndex;

        tracks.insert(tracks.begin() + toIndex, std::move(track));
    }

    // Project duration (longest track)
    double getTotalDuration() const
    {
        double maxDuration = 0.0;
        for (const auto& track : tracks)
            maxDuration = std::max(maxDuration, track.getTotalDuration());
        return maxDuration;
    }

    // Marker management
    Marker& addMarker(double time, const juce::String& name = "")
    {
        Marker marker(time, name.isEmpty() ? "Marker " + juce::String(markers.size() + 1) : name);
        markers.push_back(marker);

        // Sort markers by time
        std::sort(markers.begin(), markers.end(),
            [](const Marker& a, const Marker& b) { return a.time < b.time; });

        return markers.back();
    }

    bool removeMarker(const juce::Uuid& markerId)
    {
        auto it = std::find_if(markers.begin(), markers.end(),
            [&markerId](const Marker& m) { return m.id == markerId; });

        if (it != markers.end())
        {
            markers.erase(it);
            return true;
        }
        return false;
    }

    Marker* getMarker(const juce::Uuid& markerId)
    {
        for (auto& marker : markers)
        {
            if (marker.id == markerId)
                return &marker;
        }
        return nullptr;
    }

    Marker* getMarkerByShortcut(int shortcutNum)
    {
        for (auto& marker : markers)
        {
            if (marker.shortcutNumber == shortcutNum)
                return &marker;
        }
        return nullptr;
    }

    Marker* getNearestMarker(double time, double tolerance = 0.5)
    {
        Marker* nearest = nullptr;
        double minDist = tolerance;

        for (auto& marker : markers)
        {
            double dist = std::abs(marker.time - time);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = &marker;
            }
        }
        return nearest;
    }

    int getNumMarkers() const { return static_cast<int>(markers.size()); }
    std::vector<Marker>& getMarkers() { return markers; }
    const std::vector<Marker>& getMarkers() const { return markers; }

    // Clip group management
    juce::Uuid createGroup(const std::vector<std::pair<juce::Uuid, juce::Uuid>>& clipList,
                           const juce::String& groupName = "")
    {
        ClipGroup group(groupName.isEmpty()
            ? "Group " + juce::String(clipGroups.size() + 1)
            : groupName);

        for (const auto& [trackId, clipId] : clipList)
        {
            group.addClip(trackId, clipId);

            // Update clip's groupId
            if (auto* track = getTrack(trackId))
            {
                if (auto* clip = track->getClip(clipId))
                    clip->groupId = group.id;
            }
        }

        clipGroups.push_back(group);
        return group.id;
    }

    bool dissolveGroup(const juce::Uuid& groupId)
    {
        auto it = std::find_if(clipGroups.begin(), clipGroups.end(),
            [&groupId](const ClipGroup& g) { return g.id == groupId; });

        if (it != clipGroups.end())
        {
            // Clear groupId from all clips in the group
            for (const auto& [trackId, clipId] : it->clips)
            {
                if (auto* track = getTrack(trackId))
                {
                    if (auto* clip = track->getClip(clipId))
                        clip->groupId = juce::Uuid();
                }
            }

            clipGroups.erase(it);
            return true;
        }
        return false;
    }

    ClipGroup* getGroup(const juce::Uuid& groupId)
    {
        for (auto& group : clipGroups)
        {
            if (group.id == groupId)
                return &group;
        }
        return nullptr;
    }

    ClipGroup* getGroupForClip(const juce::Uuid& trackId, const juce::Uuid& clipId)
    {
        for (auto& group : clipGroups)
        {
            if (group.containsClip(trackId, clipId))
                return &group;
        }
        return nullptr;
    }

    int getNumGroups() const { return static_cast<int>(clipGroups.size()); }
    std::vector<ClipGroup>& getClipGroups() { return clipGroups; }
    const std::vector<ClipGroup>& getClipGroups() const { return clipGroups; }

    // Modified state
    bool isModified() const { return modified; }
    void setModified(bool state = true) { modified = state; }

    // Clear project (for new/load)
    void clear()
    {
        tracks.clear();
        markers.clear();
        clipGroups.clear();
        name = "Untitled Project";
        projectFile = juce::File();
        tempo = 120.0;
        sampleRate = 44100.0;
        timeSignatureNum = 4;
        timeSignatureDenom = 4;
        masterVolume = 1.0f;
        modified = false;
    }

    // Master volume
    float getMasterVolume() const { return masterVolume; }
    void setMasterVolume(float vol) { masterVolume = juce::jmax(0.0f, vol); }

    // Edit mode (Normal or Ripple)
    EditMode getEditMode() const { return editMode; }
    void setEditMode(EditMode mode) { editMode = mode; }
    bool isRippleEditEnabled() const { return editMode == EditMode::Ripple; }

private:
    juce::Uuid id;
    juce::String name { "Untitled Project" };
    juce::File projectFile;

    double sampleRate = 44100.0;
    double tempo = 120.0;
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;

    float masterVolume = 1.0f;
    EditMode editMode = EditMode::Normal;

    std::vector<Track> tracks;
    std::vector<Marker> markers;
    std::vector<ClipGroup> clipGroups;
    bool modified = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Project)
};
