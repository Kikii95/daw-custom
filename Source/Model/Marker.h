#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

struct Marker
{
    juce::Uuid id;
    juce::String name;
    double time = 0.0;
    juce::Colour colour = juce::Colours::yellow;
    int shortcutNumber = -1;  // 1-9 for Numpad access, -1 = no shortcut

    Marker() : id(juce::Uuid()) {}

    Marker(double t, const juce::String& n = "")
        : id(juce::Uuid())
        , name(n.isEmpty() ? "Marker" : n)
        , time(t)
    {
    }
};
