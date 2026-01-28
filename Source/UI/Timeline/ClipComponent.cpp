#include "ClipComponent.h"

ClipComponent::ClipComponent()
{
    addAndMakeVisible(waveformDisplay);
}

ClipComponent::~ClipComponent()
{
}

void ClipComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Clip background
    g.setColour(clipData.colour.withAlpha(0.8f));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Selection highlight
    if (selected)
    {
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(1), 4.0f, 2.0f);
    }

    // Clip name
    g.setColour(juce::Colours::white);
    g.setFont(11.0f);
    g.drawText(clipData.name,
               bounds.reduced(4, 2).removeFromTop(16),
               juce::Justification::centredLeft,
               true);

    // Border
    g.setColour(clipData.colour.darker(0.3f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void ClipComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(18);  // Space for name
    bounds.reduce(2, 2);
    waveformDisplay.setBounds(bounds);
}

void ClipComponent::setClipData(const Clip& clip)
{
    clipData = clip;
    waveformDisplay.setVisibleRange(clip.sourceStartOffset,
                                     clip.sourceStartOffset + clip.duration);
    repaint();
}

void ClipComponent::setThumbnail(juce::AudioThumbnail* thumb)
{
    waveformDisplay.setThumbnail(thumb);
    waveformDisplay.setWaveformColour(clipData.colour.brighter(0.3f));
}

void ClipComponent::setSelected(bool sel)
{
    if (selected != sel)
    {
        selected = sel;
        repaint();
    }
}

void ClipComponent::mouseDown(const juce::MouseEvent& e)
{
    if (onSelect)
        onSelect(this);

    dragStart = e.getPosition();
    dragging = true;
}

void ClipComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragging)
        return;

    int deltaX = e.getPosition().x - dragStart.x;

    if (onDrag && std::abs(deltaX) > 2)
    {
        onDrag(this, static_cast<double>(deltaX));
        dragStart = e.getPosition();
    }
}

void ClipComponent::mouseUp(const juce::MouseEvent& /*e*/)
{
    dragging = false;
}
