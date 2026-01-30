#pragma once

#include <juce_core/juce_core.h>
#include "Project.h"
#include "../Audio/AudioMixer.h"
#include "../Audio/AudioTrack.h"

/**
 * Handles serialization/deserialization of DAW projects to/from JSON.
 *
 * File format: .dawc (JSON)
 *
 * Serializes: Project metadata, tracks, clips, effects (native + VST3).
 */
class ProjectSerializer
{
public:
    // Version for format compatibility
    static constexpr const char* FORMAT_VERSION = "1.0";
    static constexpr const char* FILE_EXTENSION = ".dawc";

    // Serialize project to JSON string
    static juce::String toJsonString(const Project& project, const AudioMixer& mixer);

    // Save project to file
    static bool saveToFile(const juce::File& file,
                           const Project& project,
                           const AudioMixer& mixer);

    // Load project from file
    static bool loadFromFile(const juce::File& file,
                             Project& project,
                             AudioMixer& mixer,
                             juce::AudioFormatManager& formatManager);

    // Get last error message
    static juce::String getLastError() { return lastError; }

private:
    static juce::String lastError;

    // Serialization helpers
    static juce::var projectToJson(const Project& project, const AudioMixer& mixer);
    static juce::var trackToJson(const Track& track, const AudioTrack* audioTrack);
    static juce::var clipToJson(const Clip& clip);
    static juce::var effectToJson(const EffectSlot* effect);
    static juce::var markerToJson(const Marker& marker);
    static juce::var clipGroupToJson(const ClipGroup& group);

    // Deserialization helpers
    static bool loadFromJson(const juce::var& json,
                             Project& project,
                             AudioMixer& mixer,
                             juce::AudioFormatManager& formatManager);
    static bool jsonToTrack(const juce::var& json,
                            Track& track,
                            AudioTrack* audioTrack,
                            juce::AudioFormatManager& formatManager);
    static bool jsonToClip(const juce::var& json, Clip& clip);
    static bool jsonToEffect(const juce::var& json, EffectChain& chain);
    static bool jsonToMarker(const juce::var& json, Marker& marker);
    static bool jsonToClipGroup(const juce::var& json, ClipGroup& group);

    // Effect factory
    static std::unique_ptr<EffectSlot> createEffectByType(const juce::String& type);
};
