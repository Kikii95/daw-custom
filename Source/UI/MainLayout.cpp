#include "MainLayout.h"
#include "UI/Theme/AppTheme.h"

MainLayout::MainLayout()
{
    addAndMakeVisible(transportBar);
}

MainLayout::~MainLayout()
{
}

void MainLayout::resized()
{
    auto bounds = getLocalBounds();

    // Transport bar at top
    transportBar.setBounds(bounds.removeFromTop(transportBarHeight));

    // Mixer at bottom (placeholder area for now)
    mixerArea = bounds.removeFromBottom(mixerPanelHeight);

    // Timeline takes the rest
    timelineArea = bounds;
}

void MainLayout::paint(juce::Graphics& g)
{
    // Dark background
    g.fillAll(Theme::colour(Theme::bgDark));

    // Timeline area
    g.setColour(Theme::colour(Theme::bgSlot));
    g.fillRect(timelineArea);

    // Mixer area
    g.setColour(Theme::colour(Theme::bgPanel));
    g.fillRect(mixerArea);

    // Draw divider lines
    g.setColour(Theme::colour(Theme::border));
    g.drawHorizontalLine(transportBarHeight, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(getHeight() - mixerPanelHeight, 0.0f, static_cast<float>(getWidth()));
}
