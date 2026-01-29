#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TimelineRuler.h"
#include "TrackLane.h"
#include "UI/Waveform/WaveformCache.h"
#include "Audio/TransportController.h"
#include "Model/Project.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <memory>

class TimelinePanel : public juce::Component,
                      public juce::Timer,
                      public TransportController::Listener
{
public:
    TimelinePanel();
    ~TimelinePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Project binding
    void setProject(Project* proj);

    // Transport binding
    void setTransportController(TransportController* controller);

    // Format manager for waveforms
    void setFormatManager(juce::AudioFormatManager* manager);

    // Zoom control
    void setPixelsPerSecond(double pps);
    double getPixelsPerSecond() const { return pixelsPerSecond; }

    void zoomIn();
    void zoomOut();

    // Scroll position
    void setScrollPosition(double timeInSeconds);
    double getScrollPosition() const { return scrollPosition; }

    // Mouse wheel for zoom/scroll
    void mouseWheelMove(const juce::MouseEvent& e,
                        const juce::MouseWheelDetails& wheel) override;

    // TransportController::Listener
    void transportStateChanged(TransportController::State newState) override;
    void transportPositionChanged(double newPosition) override;

    // Timer for playhead updates
    void timerCallback() override;

    // Refresh track lanes from project
    void refreshTracks();

    // Callbacks
    std::function<void(juce::Uuid)> onTrackSelected;

private:
    void updateVisibleRange();

    TimelineRuler ruler;
    std::vector<std::unique_ptr<TrackLane>> trackLanes;
    juce::Viewport viewport;
    juce::Component trackContainer;

    WaveformCache waveformCache;

    Project* project = nullptr;
    TransportController* transport = nullptr;
    juce::AudioFormatManager* formatManager = nullptr;

    double pixelsPerSecond = 50.0;
    double scrollPosition = 0.0;

    static constexpr int rulerHeight = 30;
    static constexpr int trackHeight = 80;
    static constexpr double minZoom = 5.0;
    static constexpr double maxZoom = 500.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelinePanel)
};
