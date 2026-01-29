#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Model/Clip.h"
#include "UI/Waveform/WaveformDisplay.h"

class ClipComponent : public juce::Component,
                      public juce::SettableTooltipClient
{
public:
    ClipComponent();
    ~ClipComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Clip data
    void setClipData(const Clip& clip);
    const Clip& getClipData() const { return clipData; }

    // Waveform
    void setThumbnail(juce::AudioThumbnail* thumb);

    // Selection
    void setSelected(bool selected);
    bool isSelected() const { return selected; }

    // Mouse handling
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    // Callback for drag
    std::function<void(ClipComponent*, double)> onDrag;           // Horizontal drag (delta pixels)
    std::function<void(ClipComponent*, int)> onDragToNewTrack;    // Vertical drag (target track index)
    std::function<void(ClipComponent*, bool addToSelection)> onSelect;  // addToSelection = Ctrl+click
    std::function<void(ClipComponent*)> onDragStart;              // Called when drag begins
    std::function<void(ClipComponent*)> onDragEnd;                // Called when drag ends

    // Context menu callbacks
    std::function<void(ClipComponent*)> onDelete;
    std::function<void(ClipComponent*)> onDuplicate;

    // Trim callback: (clip, isLeftEdge, deltaPixels)
    std::function<void(ClipComponent*, bool, double)> onTrim;

    // Trim edge detection
    enum class EdgeHover { None, Left, Right };
    EdgeHover getHoveredEdge() const { return hoveredEdge; }

private:
    static constexpr int edgeHitZone = 8;  // pixels from edge to detect trim
    Clip clipData;
    WaveformDisplay waveformDisplay;
    bool selected = false;
    bool hovered = false;
    bool dragging = false;
    bool verticalDragActive = false;

    juce::Point<int> dragStart;
    juce::Point<int> dragStartScreen;  // For vertical drag detection
    int initialTrackY = 0;              // Y center of original track

    // Trim state
    EdgeHover hoveredEdge = EdgeHover::None;
    bool trimming = false;
    EdgeHover trimEdge = EdgeHover::None;
    double trimStartValue = 0.0;  // Original start time or duration

    void updateMouseCursor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};
