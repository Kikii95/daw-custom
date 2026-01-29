#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TimelineRuler : public juce::Component
{
public:
    TimelineRuler();

    void paint(juce::Graphics& g) override;

    // Timeline settings
    void setVisibleRange(double startTime, double endTime);
    void setPixelsPerSecond(double pps) { pixelsPerSecond = pps; repaint(); }
    double getPixelsPerSecond() const { return pixelsPerSecond; }

    // Playhead position
    void setPlayheadPosition(double timeInSeconds);

    // Tempo for beat grid
    void setTempo(double bpm) { tempo = bpm; repaint(); }

    // Header offset for alignment with track lanes
    void setHeaderOffset(int offset) { headerOffset = offset; repaint(); }

    // Loop markers
    void setLoopEnabled(bool enabled);
    void setLoopRange(double start, double end);
    bool isLoopEnabled() const { return loopEnabled; }
    double getLoopStart() const { return loopStart; }
    double getLoopEnd() const { return loopEnd; }

    // Mouse handling for loop markers
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    // Callbacks
    std::function<void(double, double)> onLoopRangeChanged;
    std::function<void(double)> onPositionClicked;

private:
    double visibleStart = 0.0;
    double visibleEnd = 30.0;
    double pixelsPerSecond = 50.0;
    double playheadPosition = 0.0;
    double tempo = 120.0;
    int headerOffset = 0;

    // Loop state
    bool loopEnabled = false;
    double loopStart = 0.0;
    double loopEnd = 0.0;

    // Drag state for loop markers
    enum class DragTarget { None, LoopStart, LoopEnd, LoopRegion };
    DragTarget dragTarget = DragTarget::None;
    double dragStartTime = 0.0;
    double dragLoopStartOffset = 0.0;

    // Draw helpers
    void drawTimeTicks(juce::Graphics& g);
    void drawPlayhead(juce::Graphics& g);
    void drawLoopMarkers(juce::Graphics& g);

    // Helpers
    double xToTime(int x) const;
    int timeToX(double time) const;
    DragTarget hitTestLoopMarker(int x) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineRuler)
};
