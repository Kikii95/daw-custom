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

    // Marquee selection rectangle
    if (isMarqueeSelecting && !marqueeRect.isEmpty())
    {
        g.setColour(Theme::Colours::accent().withAlpha(0.2f));
        g.fillRect(marqueeRect);
        g.setColour(Theme::Colours::accent());
        g.drawRect(marqueeRect, 1);
    }
}

void TimelinePanel::paintOverChildren(juce::Graphics& g)
{
    // Draw ghost clip preview during drag
    if (isDraggingClip && !ghostClipBounds.empty())
    {
        for (size_t i = 0; i < ghostClipBounds.size(); ++i)
        {
            auto bounds = ghostClipBounds[i].toFloat();
            auto colour = (i < ghostClipColours.size()) ? ghostClipColours[i] : Theme::Colours::accent();

            // Semi-transparent fill
            g.setColour(colour.withAlpha(0.4f));
            g.fillRoundedRectangle(bounds, Theme::cornerRadiusSm);

            // Accent border
            g.setColour(Theme::Colours::accent().withAlpha(0.8f));
            g.drawRoundedRectangle(bounds, Theme::cornerRadiusSm, 2.0f);
        }
    }

    // Draw snap indicator line
    if (showSnapLine && snapLineX > 0)
    {
        // Vertical line from ruler to bottom
        g.setColour(Theme::Colours::accent().withAlpha(0.9f));
        g.drawVerticalLine(snapLineX, static_cast<float>(rulerHeight), static_cast<float>(getHeight()));

        // Small marker triangle at top
        juce::Path marker;
        marker.addTriangle(
            static_cast<float>(snapLineX - 5), static_cast<float>(rulerHeight),
            static_cast<float>(snapLineX + 5), static_cast<float>(rulerHeight),
            static_cast<float>(snapLineX), static_cast<float>(rulerHeight + 8)
        );
        g.setColour(Theme::Colours::accent());
        g.fillPath(marker);
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
    ruler.setProject(proj);
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
    targetPixelsPerSecond = juce::jlimit(minZoom, maxZoom, targetPixelsPerSecond * 1.5);
    isZoomAnimating = true;
}

void TimelinePanel::zoomOut()
{
    targetPixelsPerSecond = juce::jlimit(minZoom, maxZoom, targetPixelsPerSecond / 1.5);
    isZoomAnimating = true;
}

void TimelinePanel::zoomToSelection()
{
    if (selectedClips.empty() || project == nullptr)
        return;

    double minTime = std::numeric_limits<double>::max();
    double maxTime = 0.0;

    // Find time bounds of selected clips
    for (const auto& [trackId, clipId] : selectedClips)
    {
        if (auto* track = project->getTrack(trackId))
        {
            if (auto* clip = track->getClip(clipId))
            {
                minTime = juce::jmin(minTime, clip->startTime);
                maxTime = juce::jmax(maxTime, clip->getEndTime());
            }
        }
    }

    if (minTime >= maxTime)
        return;

    // Add 10% padding on each side
    double duration = maxTime - minTime;
    double padding = duration * 0.1;
    zoomToTimeRange(minTime - padding, maxTime + padding);
}

void TimelinePanel::zoomToFitAll()
{
    if (project == nullptr)
        return;

    double maxEndTime = 0.0;

    // Find the latest clip end time across all tracks
    for (const auto& track : project->getTracks())
    {
        for (const auto& clip : track.clips)
            maxEndTime = juce::jmax(maxEndTime, clip.getEndTime());
    }

    // Minimum view of 10 seconds if empty or very short
    maxEndTime = juce::jmax(maxEndTime, 10.0);

    // Add 5% padding at the end
    zoomToTimeRange(0.0, maxEndTime * 1.05);
}

void TimelinePanel::zoomToTimeRange(double startTime, double endTime)
{
    if (endTime <= startTime)
        return;

    double duration = endTime - startTime;

    // Calculate available width (excluding header/ruler margins)
    const int trackHeaderWidth = 120;  // Approximate header width
    const int availableWidth = getWidth() - trackHeaderWidth;

    if (availableWidth <= 0)
        return;

    // Calculate pixels per second to fit the range
    double newPps = static_cast<double>(availableWidth) / duration;
    targetPixelsPerSecond = juce::jlimit(minZoom, maxZoom, newPps);
    isZoomAnimating = true;

    // Set scroll position to start of range
    scrollPosition = juce::jmax(0.0, startTime);
    updateVisibleRange();
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

    // Smooth zoom animation
    if (isZoomAnimating)
    {
        constexpr double smoothingFactor = 0.15;  // Adjust for faster/slower animation
        double diff = targetPixelsPerSecond - pixelsPerSecond;

        if (std::abs(diff) < 0.5)  // Close enough, snap to target
        {
            pixelsPerSecond = targetPixelsPerSecond;
            isZoomAnimating = false;
        }
        else
        {
            pixelsPerSecond += diff * smoothingFactor;
        }

        // Update display
        ruler.setPixelsPerSecond(pixelsPerSecond);
        for (auto& lane : trackLanes)
            lane->setPixelsPerSecond(pixelsPerSecond);
        updateVisibleRange();
        repaint();
    }
}

void TimelinePanel::refreshTracks()
{
    trackLanes.clear();

    if (project == nullptr)
        return;

    const auto& tracks = project->getTracks();
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& track = tracks[i];
        auto lane = std::make_unique<TrackLane>();
        lane->setTrackData(track);
        lane->setTrackIndex(static_cast<int>(i));  // Set track index for drag-drop
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

        // Wire clip inter-track dragging - now uses absolute target track index
        lane->onClipDraggedToTrack = [this](TrackLane* fromLane, ClipComponent* clipComp, int targetTrackIndex)
        {
            auto trackId = fromLane->getTrackData().id;
            auto clipId = clipComp->getClipData().id;
            int fromIdx = fromLane->getTrackIndex();

            // Validate target
            if (targetTrackIndex < 0 || targetTrackIndex >= static_cast<int>(trackLanes.size()))
                return;
            if (targetTrackIndex == fromIdx)
                return;

            // Check if this clip is part of a multi-selection
            if (selectedClips.size() > 1 && isClipSelected(trackId, clipId))
            {
                // Move all selected clips - calculate delta from absolute indices
                int trackDelta = targetTrackIndex - fromIdx;
                moveSelectedClipsToTrack(trackDelta);
            }
            else
            {
                // Single clip move - use absolute target index directly
                if (onClipMoved)
                    onClipMoved(clipId, trackId, fromIdx, targetTrackIndex);
            }

            // Update the clip's current track index after move
            clipComp->currentTrackIndex = targetTrackIndex;
        };

        // Query track index at screen Y position (for free drag to any track)
        lane->onQueryTrackAtScreenY = [this](int screenY) -> int
        {
            auto localY = getLocalPoint(nullptr, juce::Point<int>(0, screenY)).y;
            return getTrackIndexAtY(localY);
        };

        // Set drop target visual feedback
        lane->onSetDropTargetTrack = [this](int targetTrackIndex)
        {
            setDropTargetTrack(targetTrackIndex);
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

        // Wire multi-clip drag
        lane->onClipDragDelta = [this](TrackLane* trackLane, ClipComponent* clipComp, double deltaPixels)
        {
            auto trackId = trackLane->getTrackData().id;
            auto clipId = clipComp->getClipData().id;

            // Check if this clip is part of a multi-selection
            if (selectedClips.size() > 1 && isClipSelected(trackId, clipId))
            {
                // Move all selected clips together
                moveSelectedClips(deltaPixels);

                // Update ghost positions for all selected clips
                if (isDraggingClip)
                {
                    ghostClipBounds.clear();
                    for (const auto& [selTrackId, selClipId] : selectedClips)
                    {
                        for (auto& l : trackLanes)
                        {
                            if (l->getTrackData().id == selTrackId)
                            {
                                auto laneY = l->getY() + rulerHeight;
                                for (int i = 0; i < l->getNumChildComponents(); ++i)
                                {
                                    if (auto* cc = dynamic_cast<ClipComponent*>(l->getChildComponent(i)))
                                    {
                                        if (cc->getClipData().id == selClipId)
                                        {
                                            ghostClipBounds.push_back(cc->getBounds().translated(0, laneY));
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                    repaint();
                }
            }
            else
            {
                // Single clip drag - move only this clip
                double deltaTime = deltaPixels / pixelsPerSecond;
                Clip data = clipComp->getClipData();
                double newTime = juce::jmax(0.0, data.startTime + deltaTime);

                bool shiftHeld = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
                if (snapEnabled && !shiftHeld)
                {
                    newTime = std::round(newTime / snapInterval) * snapInterval;

                    // Show snap indicator line
                    showSnapLine = true;
                    snapLineX = TrackLane::getHeaderWidth() +
                                static_cast<int>((newTime - scrollPosition) * pixelsPerSecond);
                }
                else
                {
                    showSnapLine = false;
                }

                data.startTime = newTime;
                clipComp->setClipData(data);
                trackLane->repaint();

                // Update ghost position
                if (isDraggingClip && !ghostClipBounds.empty())
                {
                    auto laneY = trackLane->getY() + rulerHeight;
                    ghostClipBounds[0] = clipComp->getBounds().translated(0, laneY);
                }
                repaint();

                // Notify for persistence
                if (onClipTimeMoved)
                    onClipTimeMoved(trackId, clipId, newTime);
            }
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

        // Wire clip trim callback
        lane->onClipTrim = [this](TrackLane* trackLane, ClipComponent* clipComp,
                                   bool isLeftEdge, double deltaPixels)
        {
            double deltaTime = deltaPixels / pixelsPerSecond;
            Clip data = clipComp->getClipData();

            bool shiftHeld = juce::ModifierKeys::getCurrentModifiers().isShiftDown();

            if (isLeftEdge)
            {
                // Trim left: adjust startTime, sourceStartOffset, duration
                double newStart = juce::jmax(0.0, data.startTime + deltaTime);
                if (snapEnabled && !shiftHeld)
                    newStart = std::round(newStart / getSnapIntervalFromBeat()) * getSnapIntervalFromBeat();

                double startDelta = newStart - data.startTime;
                double newDuration = data.duration - startDelta;

                // Prevent trimming past end or into negative sourceOffset
                if (newDuration > 0.1 && (data.sourceStartOffset + startDelta) >= 0.0)
                {
                    data.startTime = newStart;
                    data.sourceStartOffset += startDelta;
                    data.duration = newDuration;
                }
            }
            else
            {
                // Trim right: adjust duration only
                double newDuration = juce::jmax(0.1, data.duration + deltaTime);
                if (snapEnabled && !shiftHeld)
                {
                    double newEnd = data.startTime + newDuration;
                    newEnd = std::round(newEnd / getSnapIntervalFromBeat()) * getSnapIntervalFromBeat();
                    newDuration = newEnd - data.startTime;
                }
                data.duration = juce::jmax(0.1, newDuration);
            }

            clipComp->setClipData(data);
            trackLane->resized();  // Re-layout clips

            // Notify for model update
            if (onClipTrimmed)
                onClipTrimmed(trackLane->getTrackData().id, data.id,
                              data.startTime, data.duration, data.sourceStartOffset);
        };

        // Wire fade changed callback
        lane->onClipFadeChanged = [this](TrackLane* trackLane, ClipComponent* clipComp,
                                          bool isFadeIn, double duration)
        {
            if (onClipFadeChanged)
                onClipFadeChanged(trackLane->getTrackData().id, clipComp->getClipData().id,
                                  isFadeIn, duration);
        };

        // Wire drag ghost preview
        lane->onClipDragStart = [this](TrackLane* trackLane, ClipComponent* clipComp)
        {
            isDraggingClip = true;
            ghostClipBounds.clear();
            ghostClipColours.clear();
            dragStartPositions.clear();

            auto trackId = trackLane->getTrackData().id;
            auto clipId = clipComp->getClipData().id;

            // Store starting positions for undo system
            if (selectedClips.size() > 1 && isClipSelected(trackId, clipId))
            {
                // Multi-selection: store all selected clips
                for (const auto& [selTrackId, selClipId] : selectedClips)
                {
                    for (auto& tl : trackLanes)
                    {
                        if (tl->getTrackData().id == selTrackId)
                        {
                            for (int i = 0; i < tl->getNumChildComponents(); ++i)
                            {
                                if (auto* cc = dynamic_cast<ClipComponent*>(tl->getChildComponent(i)))
                                {
                                    if (cc->getClipData().id == selClipId)
                                    {
                                        dragStartPositions.push_back({selTrackId, selClipId,
                                                                       cc->getClipData().startTime, 0.0});
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
            else
            {
                // Single clip
                dragStartPositions.push_back({trackId, clipId, clipComp->getClipData().startTime, 0.0});
            }

            // If this clip is part of a multi-selection, show ghosts for all selected
            if (selectedClips.size() > 1 && isClipSelected(trackId, clipId))
            {
                // Add ghost for each selected clip
                for (const auto& [selTrackId, selClipId] : selectedClips)
                {
                    for (auto& tl : trackLanes)
                    {
                        if (tl->getTrackData().id == selTrackId)
                        {
                            auto tlY = tl->getY() + rulerHeight;
                            for (int i = 0; i < tl->getNumChildComponents(); ++i)
                            {
                                if (auto* cc = dynamic_cast<ClipComponent*>(tl->getChildComponent(i)))
                                {
                                    if (cc->getClipData().id == selClipId)
                                    {
                                        ghostClipBounds.push_back(cc->getBounds().translated(0, tlY));
                                        ghostClipColours.push_back(cc->getClipData().colour);
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
            else
            {
                // Single clip ghost
                auto laneY = trackLane->getY() + rulerHeight;
                ghostClipBounds.push_back(clipComp->getBounds().translated(0, laneY));
                ghostClipColours.push_back(clipComp->getClipData().colour);
            }
            repaint();
        };

        lane->onClipDragEnd = [this](TrackLane* trackLane, ClipComponent* /*clipComp*/)
        {
            isDraggingClip = false;
            ghostClipBounds.clear();
            ghostClipColours.clear();
            showSnapLine = false;
            clearDropTargets();  // Clear visual feedback on drag end

            // Capture final positions and call undo callback
            if (onClipMoveComplete && !dragStartPositions.empty())
            {
                // Update final positions from current clip data
                for (auto& pos : dragStartPositions)
                {
                    for (auto& tl : trackLanes)
                    {
                        if (tl->getTrackData().id == pos.trackId)
                        {
                            for (int i = 0; i < tl->getNumChildComponents(); ++i)
                            {
                                if (auto* cc = dynamic_cast<ClipComponent*>(tl->getChildComponent(i)))
                                {
                                    if (cc->getClipData().id == pos.clipId)
                                    {
                                        pos.newStartTime = cc->getClipData().startTime;
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }

                // Only call if positions actually changed
                bool anyChange = false;
                for (const auto& pos : dragStartPositions)
                {
                    if (std::abs(pos.oldStartTime - pos.newStartTime) > 0.001)
                    {
                        anyChange = true;
                        break;
                    }
                }

                if (anyChange)
                    onClipMoveComplete(dragStartPositions);
            }

            // Detect and apply crossfades on the track where clip was dropped
            detectAndApplyCrossfades(trackLane);

            dragStartPositions.clear();
            repaint();
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

bool TimelinePanel::hasRecentClick() const
{
    // Consider click "recent" if within last 5 seconds
    juce::int64 now = juce::Time::currentTimeMillis();
    return (now - lastClickTimestamp) < 5000;
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

// DragAndDropTarget implementation (for internal drag from AssetBrowser)
bool TimelinePanel::isInterestedInDragSource(const SourceDetails& details)
{
    auto desc = details.description.toString();
    // Accept "AudioFile" description or absolute file paths
    return desc == "AudioFile" || juce::File::isAbsolutePath(desc);
}

void TimelinePanel::itemDragEnter(const SourceDetails& details)
{
    // Show visual feedback when drag enters
    auto localPos = getLocalPoint(details.sourceComponent, details.localPosition);
    int trackIndex = getTrackIndexAtY(localPos.y);
    setDropTargetTrack(trackIndex);
}

void TimelinePanel::itemDragMove(const SourceDetails& details)
{
    // Update visual feedback as drag moves
    auto localPos = getLocalPoint(details.sourceComponent, details.localPosition);
    int trackIndex = getTrackIndexAtY(localPos.y);
    setDropTargetTrack(trackIndex);
}

void TimelinePanel::itemDragExit(const SourceDetails& details)
{
    juce::ignoreUnused(details);
    clearDropTargets();
}

void TimelinePanel::itemDropped(const SourceDetails& details)
{
    clearDropTargets();

    auto localPos = getLocalPoint(details.sourceComponent, details.localPosition);
    int trackIndex = getTrackIndexAtY(localPos.y);
    double dropTime = getTimeAtX(localPos.x);

    juce::File file;
    auto desc = details.description.toString();

    if (desc == "AudioFile")
    {
        // Get file from FileTreeComponent (sourceComponent is WeakReference)
        if (auto* fileTree = dynamic_cast<juce::FileTreeComponent*>(details.sourceComponent.get()))
            file = fileTree->getSelectedFile();
    }
    else if (juce::File::isAbsolutePath(desc))
    {
        // File path passed directly in description
        file = juce::File(desc);
    }

    if (file.existsAsFile() && onFileDropped)
        onFileDropped(file, trackIndex, dropTime);
}

void TimelinePanel::clearClipSelection()
{
    // Deselect all clips visually
    for (auto& lane : trackLanes)
    {
        for (int i = 0; i < lane->getNumChildComponents(); ++i)
        {
            if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
                clipComp->setSelected(false);
        }
    }
    selectedClips.clear();
}

void TimelinePanel::selectAllClipsOnTrack(juce::Uuid trackId)
{
    for (auto& lane : trackLanes)
    {
        if (lane->getTrackData().id == trackId)
        {
            // Select all clips visually and add to selection list
            for (int i = 0; i < lane->getNumChildComponents(); ++i)
            {
                if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
                {
                    clipComp->setSelected(true);
                    selectedClips.push_back(std::make_pair(trackId, clipComp->getClipData().id));
                }
            }
            break;
        }
    }
}

void TimelinePanel::selectAllClips()
{
    clearClipSelection();

    for (auto& lane : trackLanes)
    {
        auto trackId = lane->getTrackData().id;
        for (int i = 0; i < lane->getNumChildComponents(); ++i)
        {
            if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
            {
                clipComp->setSelected(true);
                selectedClips.push_back(std::make_pair(trackId, clipComp->getClipData().id));
            }
        }
    }
}

void TimelinePanel::addToSelection(juce::Uuid trackId, juce::Uuid clipId)
{
    // Don't add duplicates
    auto pair = std::make_pair(trackId, clipId);
    if (std::find(selectedClips.begin(), selectedClips.end(), pair) != selectedClips.end())
        return;

    selectedClips.push_back(pair);

    // Update visual state
    for (auto& lane : trackLanes)
    {
        if (lane->getTrackData().id == trackId)
        {
            for (int i = 0; i < lane->getNumChildComponents(); ++i)
            {
                if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
                {
                    if (clipComp->getClipData().id == clipId)
                    {
                        clipComp->setSelected(true);
                        break;
                    }
                }
            }
            break;
        }
    }
}

void TimelinePanel::splitSelectedClipsAtPlayhead()
{
    if (!transport || selectedClips.empty())
        return;

    double playheadTime = transport->getPosition();

    // Collect clips to split (don't modify while iterating)
    std::vector<std::tuple<juce::Uuid, juce::Uuid, double>> clipsToSplit;

    for (const auto& [trackId, clipId] : selectedClips)
    {
        // Find the clip component
        for (auto& lane : trackLanes)
        {
            if (lane->getTrackData().id == trackId)
            {
                for (int i = 0; i < lane->getNumChildComponents(); ++i)
                {
                    if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
                    {
                        const auto& clipData = clipComp->getClipData();
                        if (clipData.id == clipId)
                        {
                            // Check if playhead is within this clip (not at edges)
                            if (playheadTime > clipData.startTime + 0.01 &&
                                playheadTime < clipData.getEndTime() - 0.01)
                            {
                                clipsToSplit.push_back({trackId, clipId, playheadTime});
                            }
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    // Trigger split callback for each clip
    for (const auto& [trackId, clipId, splitTime] : clipsToSplit)
    {
        if (onClipSplit)
            onClipSplit(trackId, clipId, splitTime);
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

void TimelinePanel::setTempo(double bpm)
{
    tempo = juce::jlimit(20.0, 999.0, bpm);
    ruler.setTempo(tempo);

    // Recalculate snap interval if using beat-based snap
    if (currentSnap != SnapValue::Off)
    {
        snapInterval = getSnapIntervalFromBeat();
        for (auto& lane : trackLanes)
            lane->setSnapInterval(snapInterval);
    }

    repaint();
}

void TimelinePanel::setSnapValue(SnapValue value)
{
    currentSnap = value;

    if (value == SnapValue::Off)
    {
        snapEnabled = false;
        for (auto& lane : trackLanes)
            lane->setSnapEnabled(false);
    }
    else
    {
        snapEnabled = true;
        snapInterval = getSnapIntervalFromBeat();
        for (auto& lane : trackLanes)
        {
            lane->setSnapEnabled(true);
            lane->setSnapInterval(snapInterval);
        }
    }
}

double TimelinePanel::getSnapIntervalFromBeat() const
{
    double beatDuration = 60.0 / tempo;

    switch (currentSnap)
    {
        case SnapValue::Bar:       return beatDuration * timeSignatureNum;
        case SnapValue::Beat:      return beatDuration;
        case SnapValue::Half:      return beatDuration / 2.0;
        case SnapValue::Quarter:   return beatDuration / 4.0;
        case SnapValue::Eighth:    return beatDuration / 8.0;
        case SnapValue::Sixteenth: return beatDuration / 16.0;
        default: return 0.0;
    }
}

void TimelinePanel::setTimeSignature(int numerator, int denominator)
{
    timeSignatureNum = numerator;
    timeSignatureDenom = denominator;
    ruler.setTimeSignature(numerator, denominator);
    repaint();
}

bool TimelinePanel::isClipSelected(juce::Uuid trackId, juce::Uuid clipId) const
{
    auto pair = std::make_pair(trackId, clipId);
    return std::find(selectedClips.begin(), selectedClips.end(), pair) != selectedClips.end();
}

void TimelinePanel::moveSelectedClips(double deltaPixels)
{
    if (selectedClips.empty())
        return;

    double deltaTime = deltaPixels / pixelsPerSecond;
    bool shiftHeld = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
    double firstClipSnappedTime = -1.0;

    // Move each selected clip
    for (const auto& [trackId, clipId] : selectedClips)
    {
        // Find the track lane
        for (auto& lane : trackLanes)
        {
            if (lane->getTrackData().id == trackId)
            {
                // Find the clip component in this lane
                for (int i = 0; i < lane->getNumChildComponents(); ++i)
                {
                    if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
                    {
                        if (clipComp->getClipData().id == clipId)
                        {
                            Clip data = clipComp->getClipData();
                            double newTime = juce::jmax(0.0, data.startTime + deltaTime);

                            if (snapEnabled && !shiftHeld)
                            {
                                newTime = std::round(newTime / snapInterval) * snapInterval;

                                // Track first clip's snap position for indicator
                                if (firstClipSnappedTime < 0.0)
                                    firstClipSnappedTime = newTime;
                            }

                            data.startTime = newTime;
                            clipComp->setClipData(data);

                            // Notify for persistence
                            if (onClipTimeMoved)
                                onClipTimeMoved(trackId, clipId, newTime);

                            break;
                        }
                    }
                }
                lane->repaint();
                break;
            }
        }
    }

    // Update snap line indicator
    if (snapEnabled && !shiftHeld && firstClipSnappedTime >= 0.0)
    {
        showSnapLine = true;
        snapLineX = TrackLane::getHeaderWidth() +
                    static_cast<int>((firstClipSnappedTime - scrollPosition) * pixelsPerSecond);
    }
    else
    {
        showSnapLine = false;
    }
}

void TimelinePanel::moveSelectedClipsToTrack(int trackDelta)
{
    if (selectedClips.empty() || trackDelta == 0)
        return;

    // Create a copy since we'll be modifying the structure
    auto clipsToMove = selectedClips;

    for (const auto& [trackId, clipId] : clipsToMove)
    {
        // Find source track index
        int fromIdx = -1;
        for (int i = 0; i < static_cast<int>(trackLanes.size()); ++i)
        {
            if (trackLanes[static_cast<size_t>(i)]->getTrackData().id == trackId)
            {
                fromIdx = i;
                break;
            }
        }

        if (fromIdx < 0)
            continue;

        int toIdx = fromIdx + trackDelta;
        if (toIdx < 0 || toIdx >= static_cast<int>(trackLanes.size()))
            continue;

        // Notify to move the clip
        if (onClipMoved)
            onClipMoved(clipId, trackId, fromIdx, toIdx);
    }
}

void TimelinePanel::mouseDown(const juce::MouseEvent& e)
{
    // Record click position for paste operations
    lastClickTime = getTimeAtX(e.getPosition().x);
    lastClickTrackIndex = getTrackIndexAtY(e.getPosition().y);
    lastClickTimestamp = juce::Time::currentTimeMillis();

    // Only start marquee selection if click is in track container area and not on a clip
    auto trackArea = getLocalBounds().withTrimmedTop(rulerHeight);

    if (e.mods.isLeftButtonDown() && trackArea.contains(e.getPosition()))
    {
        // Clear existing selection unless Shift is held
        if (!e.mods.isShiftDown())
            clearClipSelection();

        isMarqueeSelecting = true;
        marqueeStart = e.getPosition();
        marqueeRect = juce::Rectangle<int>(marqueeStart, marqueeStart);
    }
}

void TimelinePanel::mouseDrag(const juce::MouseEvent& e)
{
    if (isMarqueeSelecting)
    {
        marqueeRect = juce::Rectangle<int>(marqueeStart, e.getPosition());

        // Select clips that intersect the marquee
        selectClipsInRect(marqueeRect);

        repaint();
    }
}

void TimelinePanel::mouseUp(const juce::MouseEvent& /*e*/)
{
    if (isMarqueeSelecting)
    {
        isMarqueeSelecting = false;
        marqueeRect = {};
        repaint();
    }
}

void TimelinePanel::selectClipsInRect(const juce::Rectangle<int>& rect)
{
    // Convert rect from panel coordinates to check against clips
    auto normalizedRect = rect.toFloat();

    // First, clear selection (unless Shift is held - handled in mouseDown)
    for (auto& lane : trackLanes)
    {
        for (int i = 0; i < lane->getNumChildComponents(); ++i)
        {
            if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
                clipComp->setSelected(false);
        }
    }
    selectedClips.clear();

    // Check each clip against the rectangle
    for (auto& lane : trackLanes)
    {
        auto laneY = lane->getY() + rulerHeight;
        auto trackId = lane->getTrackData().id;

        for (int i = 0; i < lane->getNumChildComponents(); ++i)
        {
            if (auto* clipComp = dynamic_cast<ClipComponent*>(lane->getChildComponent(i)))
            {
                // Get clip bounds in panel coordinates
                auto clipBounds = clipComp->getBounds().translated(0, laneY);

                // Check if clip intersects with marquee
                if (normalizedRect.toNearestInt().intersects(clipBounds))
                {
                    clipComp->setSelected(true);
                    selectedClips.push_back(std::make_pair(trackId, clipComp->getClipData().id));
                }
            }
        }
    }
}

void TimelinePanel::detectAndApplyCrossfades(TrackLane* trackLane)
{
    if (trackLane == nullptr)
        return;

    auto trackId = trackLane->getTrackData().id;

    // Collect all clips on this track
    std::vector<ClipComponent*> clips;
    for (int i = 0; i < trackLane->getNumChildComponents(); ++i)
    {
        if (auto* clipComp = dynamic_cast<ClipComponent*>(trackLane->getChildComponent(i)))
            clips.push_back(clipComp);
    }

    if (clips.size() < 2)
        return;

    // Sort by start time
    std::sort(clips.begin(), clips.end(), [](ClipComponent* a, ClipComponent* b) {
        return a->getClipData().startTime < b->getClipData().startTime;
    });

    // Check adjacent pairs for overlaps
    for (size_t i = 0; i < clips.size() - 1; ++i)
    {
        auto* clipA = clips[i];
        auto* clipB = clips[i + 1];

        const auto& dataA = clipA->getClipData();
        const auto& dataB = clipB->getClipData();

        double endA = dataA.getEndTime();
        double startB = dataB.startTime;
        double overlapDuration = endA - startB;

        // Apply crossfade if overlap is between 0 and 2 seconds
        if (overlapDuration > 0.01 && overlapDuration < 2.0)
        {
            // Set fade out on clip A (ending clip)
            Clip updatedA = dataA;
            updatedA.fadeOutDuration = overlapDuration;
            clipA->setClipData(updatedA);

            if (onClipFadeChanged)
                onClipFadeChanged(trackId, dataA.id, false, overlapDuration);

            // Set fade in on clip B (starting clip)
            Clip updatedB = dataB;
            updatedB.fadeInDuration = overlapDuration;
            clipB->setClipData(updatedB);

            if (onClipFadeChanged)
                onClipFadeChanged(trackId, dataB.id, true, overlapDuration);
        }
    }
}
