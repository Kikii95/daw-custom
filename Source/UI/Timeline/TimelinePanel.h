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

// Snap value based on beats
enum class SnapValue { Off, Bar, Beat, Half, Quarter, Eighth, Sixteenth };

class TimelinePanel : public juce::Component,
                      public juce::Timer,
                      public TransportController::Listener
{
public:
    TimelinePanel();
    ~TimelinePanel() override;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
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

    // Marquee selection
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // TransportController::Listener
    void transportStateChanged(TransportController::State newState) override;
    void transportPositionChanged(double newPosition) override;
    void transportLoopChanged(bool enabled, double start, double end) override;

    // Timer for playhead updates
    void timerCallback() override;

    // Refresh track lanes from project
    void refreshTracks();

    // Callbacks
    std::function<void(juce::Uuid)> onTrackSelected;
    std::function<void(juce::Uuid, const juce::String&)> onTrackRenamed;
    std::function<void(juce::Uuid, juce::Uuid, int, int)> onClipMoved;  // (clipId, fromTrackId, fromTrackIdx, toTrackIdx)
    std::function<void(juce::Uuid, juce::Uuid, double)> onClipTimeMoved;  // (trackId, clipId, newStartTime)
    std::function<void(juce::Uuid)> onTrackDelete;
    std::function<void(juce::Uuid, juce::Colour)> onTrackColourChanged;
    std::function<void(juce::Uuid, juce::Uuid)> onClipDelete;      // (trackId, clipId)
    std::function<void(juce::Uuid, juce::Uuid)> onClipDuplicate;   // (trackId, clipId)
    std::function<void(double, double)> onLoopRangeChanged;        // (start, end)
    std::function<void(double)> onPositionClicked;                 // (time)
    std::function<void(juce::Uuid, juce::Uuid, double)> onClipSplit;  // (trackId, clipId, splitTime)
    std::function<void(juce::Uuid, juce::Uuid, double, double, double)> onClipTrimmed;  // (trackId, clipId, newStart, newDuration, newSourceOffset)

    // Selection
    const std::vector<std::pair<juce::Uuid, juce::Uuid>>& getSelectedClips() const { return selectedClips; }
    void clearClipSelection();
    void selectAllClipsOnTrack(juce::Uuid trackId);

    // Split clip
    void splitSelectedClipsAtPlayhead();

    // Multi-clip drag support
    void moveSelectedClips(double deltaPixels);
    void moveSelectedClipsToTrack(int trackDelta);
    bool isClipSelected(juce::Uuid trackId, juce::Uuid clipId) const;

    // Snap to grid
    void setSnapEnabled(bool enabled);
    bool isSnapEnabled() const { return snapEnabled; }
    void setSnapInterval(double seconds);
    double getSnapInterval() const { return snapInterval; }

    // Beat-based snap
    void setSnapValue(SnapValue value);
    SnapValue getSnapValue() const { return currentSnap; }
    double getSnapIntervalFromBeat() const;  // Calculates interval based on tempo + snap value

    // Tempo and time signature
    void setTempo(double bpm);
    double getTempo() const { return tempo; }
    void setTimeSignature(int numerator, int denominator);
    int getTimeSignatureNumerator() const { return timeSignatureNum; }
    int getTimeSignatureDenominator() const { return timeSignatureDenom; }

    // Position helpers for drag-drop
    int getTrackIndexAtY(int y) const;
    double getTimeAtX(int x) const;

    // Last click position for paste operations
    double getLastClickTime() const { return lastClickTime; }
    int getLastClickTrackIndex() const { return lastClickTrackIndex; }
    bool hasRecentClick() const;  // Returns true if click was within 5 seconds

    // Drop target management
    void setDropTargetTrack(int trackIndex);
    void clearDropTargets();

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
    double targetPixelsPerSecond = 50.0;
    bool isZoomAnimating = false;
    double scrollPosition = 0.0;

    static constexpr int rulerHeight = 30;
    static constexpr int trackHeight = 80;
    static constexpr double minZoom = 5.0;
    static constexpr double maxZoom = 500.0;

    // Snap settings
    bool snapEnabled = true;
    double snapInterval = 0.25;  // 250ms default (updated by setSnapValue)
    SnapValue currentSnap = SnapValue::Sixteenth;  // Default: 1/16 note

    // Tempo and time signature
    double tempo = 120.0;
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;

    // Multi-selection: pairs of (trackId, clipId)
    std::vector<std::pair<juce::Uuid, juce::Uuid>> selectedClips;

    // Marquee selection state
    bool isMarqueeSelecting = false;
    juce::Point<int> marqueeStart;
    juce::Rectangle<int> marqueeRect;
    void selectClipsInRect(const juce::Rectangle<int>& rect);

    // Drag ghost preview state
    bool isDraggingClip = false;
    std::vector<juce::Rectangle<int>> ghostClipBounds;  // One per selected clip
    std::vector<juce::Colour> ghostClipColours;          // Corresponding colours

    // Snap visual indicator state
    bool showSnapLine = false;
    int snapLineX = 0;

    // Last click position for paste
    double lastClickTime = 0.0;
    int lastClickTrackIndex = -1;
    juce::int64 lastClickTimestamp = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelinePanel)
};
