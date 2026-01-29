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
    void setName(const juce::String& newName);

    // Update levels for metering
    void setMeterLevels(float left, float right);

    // Callbacks
    std::function<void(juce::Uuid, float)> onVolumeChanged;
    std::function<void(juce::Uuid, float)> onPanChanged;
    std::function<void(juce::Uuid, bool)> onMuteChanged;
    std::function<void(juce::Uuid, bool)> onSoloChanged;
    std::function<void(juce::Uuid)> onSelected;
    std::function<void(int, int)> onReorder;  // (fromIndex, toIndex) for drag reorder

    // Index in mixer (for reorder)
    void setChannelIndex(int idx) { channelIndex = idx; }
    int getChannelIndex() const { return channelIndex; }

    // Selection state
    void setSelected(bool shouldBeSelected);
    bool isSelected() const { return selected; }

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    bool selected = false;
    bool hovered = false;
    bool dragging = false;
    juce::Point<int> dragStartPos;
    int dragStartX = 0;
    int channelIndex = 0;
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
