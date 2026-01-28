#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(1280, 720);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));

    g.setColour(juce::Colour(0xff16213e));
    g.fillRect(0, 0, getWidth(), 60);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("DAW Custom", 20, 15, 200, 30, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff0f3460));
    g.fillRect(0, getHeight() - 100, getWidth(), 100);

    g.setColour(juce::Colours::grey);
    g.setFont(juce::Font(14.0f));
    g.drawText("v0.1.0 - JUCE Foundation",
               getWidth() - 200, getHeight() - 30, 190, 20,
               juce::Justification::centredRight);
}

void MainComponent::resized()
{
}
