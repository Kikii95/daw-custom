#pragma once

#include "Track.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <memory>

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

    // Project duration (longest track)
    double getTotalDuration() const
    {
        double maxDuration = 0.0;
        for (const auto& track : tracks)
            maxDuration = std::max(maxDuration, track.getTotalDuration());
        return maxDuration;
    }

    // Modified state
    bool isModified() const { return modified; }
    void setModified(bool state = true) { modified = state; }

    // Clear project (for new/load)
    void clear()
    {
        tracks.clear();
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

private:
    juce::Uuid id;
    juce::String name { "Untitled Project" };
    juce::File projectFile;

    double sampleRate = 44100.0;
    double tempo = 120.0;
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;

    float masterVolume = 1.0f;

    std::vector<Track> tracks;
    bool modified = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Project)
};
