#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <memory>

struct Clip
{
    juce::Uuid id;
    juce::String name;

    // Timeline position (in seconds)
    double startTime = 0.0;
    double duration = 0.0;

    // Source audio info
    juce::File sourceFile;
    double sourceSampleRate = 0.0;

    // Audio offset within source (for trimming)
    double sourceStartOffset = 0.0;

    // Appearance
    juce::Colour colour { juce::Colours::dodgerblue };

    // Volume/gain (linear, 0.0 to 2.0+)
    float gain = 1.0f;

    // Mute state
    bool muted = false;

    // End time helper
    double getEndTime() const { return startTime + duration; }

    // Check if this clip overlaps a given time
    bool containsTime(double time) const
    {
        return time >= startTime && time < getEndTime();
    }

    // Get local position within clip (accounting for source offset)
    double getSourcePosition(double globalTime) const
    {
        return (globalTime - startTime) + sourceStartOffset;
    }
};
