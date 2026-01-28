#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
{
}

WaveformDisplay::~WaveformDisplay()
{
    if (thumbnail != nullptr)
        thumbnail->removeChangeListener(this);
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColour);

    if (thumbnail == nullptr || thumbnail->getTotalLength() <= 0)
    {
        // Draw placeholder
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawText("No audio loaded", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto bounds = getLocalBounds().toFloat();

    // Draw waveform
    g.setColour(waveformColour);
    thumbnail->drawChannels(g, bounds.toNearestInt(),
                            visibleStart, visibleEnd,
                            1.0f);

    // Draw center line
    g.setColour(waveformColour.withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()),
                         bounds.getX(), bounds.getRight());
}

void WaveformDisplay::setThumbnail(juce::AudioThumbnail* thumb)
{
    if (thumbnail != nullptr)
        thumbnail->removeChangeListener(this);

    thumbnail = thumb;

    if (thumbnail != nullptr)
    {
        thumbnail->addChangeListener(this);
        visibleEnd = thumbnail->getTotalLength();
    }

    repaint();
}

void WaveformDisplay::setVisibleRange(double startTime, double endTime)
{
    visibleStart = startTime;
    visibleEnd = endTime;
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == thumbnail)
        repaint();
}
