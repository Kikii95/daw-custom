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

private:
    double visibleStart = 0.0;
    double visibleEnd = 30.0;
    double pixelsPerSecond = 50.0;
    double playheadPosition = 0.0;
    double tempo = 120.0;

    // Draw helpers
    void drawTimeTicks(juce::Graphics& g);
    void drawPlayhead(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineRuler)
};
