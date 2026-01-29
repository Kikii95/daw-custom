#include "TimelinePanel.h"
#include "UI/Theme/AppTheme.h"

TimelinePanel::TimelinePanel()
{
    addAndMakeVisible(ruler);

    // Set ruler offset to match track lane header width
    ruler.setHeaderOffset(TrackLane::getHeaderWidth());

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
