#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Transport/TransportBar.h"

class MainLayout : public juce::Component
{
public:
    MainLayout();
    ~MainLayout() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    // Access sub-components
    TransportBar& getTransportBar() { return transportBar; }

    // Placeholder areas for future components
    juce::Rectangle<int> getTimelineArea() const { return timelineArea; }
    juce::Rectangle<int> getMixerArea() const { return mixerArea; }

private:
    // Top bar
    TransportBar transportBar;

    // Layout areas
    juce::Rectangle<int> timelineArea;
    juce::Rectangle<int> mixerArea;

    // Layout constants
    static constexpr int transportBarHeight = 60;
    static constexpr int mixerPanelHeight = 200;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLayout)
};
