#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MeterComponent.h"
#include "Model/Track.h"

class ChannelStrip : public juce::Component
{
public:
    ChannelStrip();
    ~ChannelStrip() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Track binding
    void setTrackData(const Track& track);
    juce::Uuid getTrackId() const { return trackId; }

    // Update levels for metering
    void setMeterLevels(float left, float right);

    // Callbacks
    std::function<void(juce::Uuid, float)> onVolumeChanged;
    std::function<void(juce::Uuid, float)> onPanChanged;
    std::function<void(juce::Uuid, bool)> onMuteChanged;
    std::function<void(juce::Uuid, bool)> onSoloChanged;

private:
    juce::Uuid trackId;
    juce::String trackName;
    juce::Colour trackColour;

    // Controls
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::Label nameLabel;

    // Meter
    MeterComponent meter;

    // State
    bool muted = false;
    bool solo = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};
