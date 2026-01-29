#include "TimelinePanel.h"
#include "UI/Theme/AppTheme.h"

TimelinePanel::TimelinePanel()
{
    addAndMakeVisible(ruler);

    // Set ruler offset to match track lane header width
    ruler.setHeaderOffset(TrackLane::getHeaderWidth());

    // Wire ruler loop callbacks
    ruler.onLoopRangeChanged = [this](double start, double end)
    {
        if (onLoopRangeChanged)
            onLoopRangeChanged(start, end);
    };

    ruler.onPositionClicked = [this](double time)
    {
        if (onPositionClicked)
            onPositionClicked(time);
    };

    viewport.setViewedComponent(&trackContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    startTimerHz(30);  // Playhead updates
}

TimelinePanel::~TimelinePanel()
{
    stopTimer();

    if (transport != nullptr)
        transport->removeListener(this);
}

void TimelinePanel::paint(juce::Graphics& g)
{
    g.fillAll(Theme::colour(Theme::bgDark));

    // Empty state
    if (trackLanes.empty())
    {
        g.setColour(Theme::Colours::textMuted());
        g.setFont(15.0f);
        g.drawText("Drag audio files here or use File > Import Audio",
                   getLocalBounds().withTrimmedTop(rulerHeight + 40),
                   juce::Justification::centredTop);
    }
}

void TimelinePanel::resized()
{
    auto bounds = getLocalBounds();

    // Ruler at top
    ruler.setBounds(bounds.removeFromTop(rulerHeight));

    // Track lanes in viewport
    viewport.setBounds(bounds);

    // Size track container
    int totalHeight = juce::jmax(bounds.getHeight(),
                                  static_cast<int>(trackLanes.size()) * trackHeight);
    trackContainer.setSize(bounds.getWidth(), totalHeight);

    // Layout track lanes
    int y = 0;
    for (auto& lane : trackLanes)
    {
        lane->setBounds(0, y, trackContainer.getWidth(), trackHeight);
        y += trackHeight;
    }

    updateVisibleRange();
}

void TimelinePanel::setProject(Project* proj)
{
    project = proj;
    refreshTracks();
}

void TimelinePanel::setTransportController(TransportController* controller)
{
    if (transport != nullptr)
        transport->removeListener(this);

    transport = controller;

    if (transport != nullptr)
        transport->addListener(this);
}

void TimelinePanel::setFormatManager(juce::AudioFormatManager* manager)
{
    formatManager = manager;

    for (auto& lane : trackLanes)
    {
        lane->setFormatManager(formatManager);
        lane->setWaveformCache(&waveformCache);
    }
}

void TimelinePanel::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = juce::jlimit(minZoom, maxZoom, pps);

    ruler.setPixelsPerSecond(pixelsPerSecond);

    for (auto& lane : trackLanes)
        lane->setPixelsPerSecond(pixelsPerSecond);

    updateVisibleRange();
    repaint();
}

void TimelinePanel::zoomIn()
{
    setPixelsPerSecond(pixelsPerSecond * 1.5);
}

void TimelinePanel::zoomOut()
{
    setPixelsPerSecond(pixelsPerSecond / 1.5);
}

void TimelinePanel::setScrollPosition(double timeInSeconds)
{
    scrollPosition = juce::jmax(0.0, timeInSeconds);
    updateVisibleRange();
}

void TimelinePanel::mouseWheelMove(const juce::MouseEvent& e,
                                    const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCtrlDown())
    {
        // Zoom with Ctrl + wheel
        if (wheel.deltaY > 0)
            zoomIn();
        else if (wheel.deltaY < 0)
            zoomOut();
    }
    else
    {
        // Scroll horizontally with wheel
        double deltaTime = wheel.deltaX * 2.0 / pixelsPerSecond;
        setScrollPosition(scrollPosition - deltaTime);
    }
}

void TimelinePanel::transportStateChanged(TransportController::State /*newState*/)
{
    // Nothing special needed
}

void TimelinePanel::transportPositionChanged(double /*newPosition*/)
{
    // Handled by timer
}

void TimelinePanel::transportLoopChanged(bool enabled, double start, double end)
{
    juce::MessageManager::callAsync([this, enabled, start, end]()
    {
        ruler.setLoopEnabled(enabled);
        ruler.setLoopRange(start, end);
    });
}

void TimelinePanel::timerCallback()
{
    if (transport != nullptr)
    {
        double pos = transport->getPosition();
        ruler.setPlayheadPosition(pos);

        // Auto-scroll if playhead goes off screen
        double visibleEnd = scrollPosition + getWidth() / pixelsPerSecond;
        if (pos > visibleEnd - 1.0)  // 1 second buffer
        {
            setScrollPosition(pos - 2.0);
        }
    }
}

void TimelinePanel::refreshTracks()
{
    trackLanes.clear();

    if (project == nullptr)
        return;

    for (const auto& track : project->getTracks())
    {
        auto lane = std::make_unique<TrackLane>();
        lane->setTrackData(track);
        lane->setPixelsPerSecond(pixelsPerSecond);
        lane->setFormatManager(formatManager);
        lane->setWaveformCache(&waveformCache);
        lane->setSnapEnabled(snapEnabled);
        lane->setSnapInterval(snapInterval);

        // Wire track selection
        lane->onTrackSelected = [this](TrackLane* selectedLane)
        {
            // Deselect all tracks
            for (auto& l : trackLanes)
                l->setSelected(l.get() == selectedLane);

            // Notify parent
            if (onTrackSelected)
                onTrackSelected(selectedLane->getTrackData().id);
        };

        // Wire track rename
        lane->onTrackRenamed = [this](TrackLane* renamedLane, const juce::String& newName)
        {
            if (onTrackRenamed)
                onTrackRenamed(renamedLane->getTrackData().id, newName);
        };

        // Wire clip inter-track dragging
        lane->onClipDraggedToTrack = [this](TrackLane* fromLane, ClipComponent* clipComp, int trackDelta)
        {
            // Find index of source track
            int fromIdx = -1;
            for (int i = 0; i < static_cast<int>(trackLanes.size()); ++i)
            {
                if (trackLanes[static_cast<size_t>(i)].get() == fromLane)
                {
                    fromIdx = i;
                    break;
                }
            }

            if (fromIdx < 0)
                return;

            int toIdx = fromIdx + trackDelta;
            if (toIdx < 0 || toIdx >= static_cast<int>(trackLanes.size()) || toIdx == fromIdx)
                return;

            // Notify parent to move the clip
            if (onClipMoved)
                onClipMoved(clipComp->getClipData().id, fromLane->getTrackData().id, fromIdx, toIdx);
        };

        // Wire track context menu actions
        lane->onTrackDelete = [this](TrackLane* trackLane)
        {
            if (onTrackDelete)
                onTrackDelete(trackLane->getTrackData().id);
        };

        lane->onTrackColourChanged = [this](TrackLane* trackLane, juce::Colour newColour)
        {
            if (onTrackColourChanged)
                onTrackColourChanged(trackLane->getTrackData().id, newColour);
        };

        // Wire clip context menu actions
        lane->onClipDelete = [this](TrackLane* trackLane, ClipComponent* clipComp)
        {
            if (onClipDelete)
                onClipDelete(trackLane->getTrackData().id, clipComp->getClipData().id);
        };

        lane->onClipDuplicate = [this](TrackLane* trackLane, ClipComponent* clipComp)
        {
            if (onClipDuplicate)
                onClipDuplicate(trackLane->getTrackData().id, clipComp->getClipData().id);
        };

        // Wire clip selection for multi-select
        lane->onClipSelected = [this](TrackLane* trackLane, ClipComponent* clipComp, bool addToSelection)
        {
            auto trackId = trackLane->getTrackData().id;
            auto clipId = clipComp->getClipData().id;
            auto pair = std::make_pair(trackId, clipId);

            if (addToSelection)
            {
                // Toggle selection for this clip
                auto it = std::find(selectedClips.begin(), selectedClips.end(), pair);
                if (it != selectedClips.end())
                {
                    selectedClips.erase(it);
                }
                else
                {
                    selectedClips.push_back(pair);
                }
            }
            else
            {
                // Clear selection and select only this clip
                clearClipSelection();
                selectedClips.push_back(pair);
                clipComp->setSelected(true);
            }
        };

        // Add clips
        for (const auto& clip : track.clips)
        {
            juce::AudioThumbnail* thumb = nullptr;
            if (formatManager != nullptr && clip.sourceFile.existsAsFile())
            {
                thumb = waveformCache.getThumbnail(clip.sourceFile, *formatManager);
            }
            lane->addClipComponent(clip, thumb);
        }

        trackContainer.addAndMakeVisible(*lane);
        trackLanes.push_back(std::move(lane));
    }

    resized();
}

void TimelinePanel::updateVisibleRange()
{
    double visibleDuration = getWidth() / pixelsPerSecond;
    double endTime = scrollPosition + visibleDuration;

    ruler.setVisibleRange(scrollPosition, endTime);

    for (auto& lane : trackLanes)
        lane->setVisibleRange(scrollPosition, endTime);
}

int TimelinePanel::getTrackIndexAtY(int y) const
{
    // Account for ruler height and viewport scroll position
    int relativeY = y - rulerHeight + viewport.getViewPositionY();

    if (relativeY < 0)
        return -1;

    int index = relativeY / trackHeight;

    if (index >= 0 && index < static_cast<int>(trackLanes.size()))
        return index;

    return -1;
}

double TimelinePanel::getTimeAtX(int x) const
{
    // Account for track header width
    int contentX = x - TrackLane::getHeaderWidth();

    if (contentX < 0)
        return scrollPosition;

    return scrollPosition + static_cast<double>(contentX) / pixelsPerSecond;
}

void TimelinePanel::setDropTargetTrack(int trackIndex)
{
    for (int i = 0; i < static_cast<int>(trackLanes.size()); ++i)
        trackLanes[static_cast<size_t>(i)]->setDropTarget(i == trackIndex);
}

void TimelinePanel::clearDropTargets()
{
    for (auto& lane : trackLanes)
        lane->setDropTarget(false);
}

void TimelinePanel::clearClipSelection()
{
    // Deselect all clips visually
    for (auto& lane : trackLanes)
    {
        // Iterate through the track's clips and deselect them
        // The TrackLane manages its own clip components internally
    }
    selectedClips.clear();

    // Refresh to update visual state
    for (auto& lane : trackLanes)
        lane->repaint();
}

void TimelinePanel::selectAllClipsOnTrack(juce::Uuid trackId)
{
    for (auto& lane : trackLanes)
    {
        if (lane->getTrackData().id == trackId)
        {
            for (const auto& clip : lane->getTrackData().clips)
            {
                selectedClips.push_back(std::make_pair(trackId, clip.id));
            }
            break;
        }
    }
}

void TimelinePanel::setSnapEnabled(bool enabled)
{
    snapEnabled = enabled;
    for (auto& lane : trackLanes)
        lane->setSnapEnabled(enabled);
}

void TimelinePanel::setSnapInterval(double seconds)
{
    snapInterval = juce::jmax(0.01, seconds);
    for (auto& lane : trackLanes)
        lane->setSnapInterval(snapInterval);
}
