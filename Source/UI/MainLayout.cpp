#include "MainLayout.h"

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
    g.fillAll(juce::Colour(0xff1e1e1e));

    // Timeline area placeholder
    g.setColour(juce::Colour(0xff2d2d2d));
    g.fillRect(timelineArea);

    // Mixer area placeholder
    g.setColour(juce::Colour(0xff252525));
    g.fillRect(mixerArea);

    // Draw divider lines
    g.setColour(juce::Colour(0xff3d3d3d));
    g.drawHorizontalLine(transportBarHeight, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(getHeight() - mixerPanelHeight, 0.0f, static_cast<float>(getWidth()));

    // Placeholder text
    g.setColour(juce::Colours::grey);
    g.setFont(14.0f);
    g.drawText("Timeline", timelineArea, juce::Justification::centred);
    g.drawText("Mixer", mixerArea, juce::Justification::centred);
}
