#include "TrackLane.h"

TrackLane::TrackLane()
{
}

TrackLane::~TrackLane()
{
}

void TrackLane::paint(juce::Graphics& g)
{
    // Track background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Header area
    auto headerArea = getLocalBounds().removeFromLeft(headerWidth);
    g.setColour(trackData.colour.withAlpha(0.3f));
    g.fillRect(headerArea);

    // Track name
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText(trackData.name,
               headerArea.reduced(8, 4),
               juce::Justification::centredLeft);

    // Mute/Solo indicators
    auto indicatorArea = headerArea.removeFromBottom(20).reduced(8, 2);
    if (trackData.muted)
    {
        g.setColour(juce::Colours::red.withAlpha(0.7f));
        g.fillRoundedRectangle(indicatorArea.removeFromLeft(20).toFloat(), 3.0f);
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("M", indicatorArea.withWidth(20), juce::Justification::centred);
    }

    if (trackData.solo)
    {
        g.setColour(juce::Colours::yellow.withAlpha(0.7f));
        g.fillRoundedRectangle(indicatorArea.removeFromLeft(20).toFloat(), 3.0f);
        g.setColour(juce::Colours::black);
        g.setFont(10.0f);
        g.drawText("S", indicatorArea.withWidth(20), juce::Justification::centred);
    }

    // Border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRect(getLocalBounds());
}

void TrackLane::resized()
{
    layoutClips();
}

void TrackLane::setTrackData(const Track& track)
{
    trackData = track;
    repaint();
}

void TrackLane::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = pps;
    layoutClips();
}

void TrackLane::setVisibleRange(double startTime, double endTime)
{
    visibleStart = startTime;
    visibleEnd = endTime;
    layoutClips();
}

void TrackLane::addClipComponent(const Clip& clipData, juce::AudioThumbnail* thumbnail)
{
    auto clip = std::make_unique<ClipComponent>();
    clip->setClipData(clipData);

    if (thumbnail != nullptr)
        clip->setThumbnail(thumbnail);

    // Set callbacks
    clip->onSelect = [this](ClipComponent* c)
    {
        if (selectedClip != nullptr)
            selectedClip->setSelected(false);

        selectedClip = c;
        c->setSelected(true);

        if (onClipSelected)
            onClipSelected(this, c);
    };

    clip->onDrag = [this](ClipComponent* c, double deltaPixels)
    {
        double deltaTime = deltaPixels / pixelsPerSecond;
        Clip data = c->getClipData();
        data.startTime = juce::jmax(0.0, data.startTime + deltaTime);
        c->setClipData(data);
        layoutClips();
    };

    addAndMakeVisible(*clip);
    clipComponents.push_back(std::move(clip));

    layoutClips();
}

void TrackLane::clearClips()
{
    clipComponents.clear();
    selectedClip = nullptr;
}

void TrackLane::layoutClips()
{
    auto contentArea = getLocalBounds();
    contentArea.removeFromLeft(headerWidth);

    for (auto& clip : clipComponents)
    {
        const auto& data = clip->getClipData();

        int x = static_cast<int>((data.startTime - visibleStart) * pixelsPerSecond);
        int width = static_cast<int>(data.duration * pixelsPerSecond);

        clip->setBounds(headerWidth + x, 4, width, getHeight() - 8);
    }
}
