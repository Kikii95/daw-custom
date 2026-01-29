#include "ClipComponent.h"
#include "UI/Theme/AppTheme.h"
#include "UI/Theme/DrawingHelpers.h"

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

    // Drop shadow
    DrawingHelpers::drawShadow(g, bounds,
                                selected ? Theme::Shadows::md : Theme::Shadows::sm,
                                Theme::cornerRadiusSm);

    auto innerBounds = bounds.reduced(1);

    // Background gradient
    auto bgGradient = juce::ColourGradient(
        clipData.colour.brighter(0.15f), innerBounds.getX(), innerBounds.getY(),
        clipData.colour.darker(0.2f), innerBounds.getX(), innerBounds.getBottom(),
        false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(innerBounds, Theme::cornerRadiusSm);

    // Darker header band (top 18px)
    auto headerBounds = innerBounds.removeFromTop(18);
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(headerBounds.withTrimmedBottom(-4), Theme::cornerRadiusSm);

    // Top highlight line (beveled effect)
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawHorizontalLine(static_cast<int>(bounds.getY() + 2),
                         bounds.getX() + 4, bounds.getRight() - 4);

    // Clip name with subtle text shadow
    auto textBounds = bounds.reduced(4, 2).removeFromTop(16);
    g.setFont(juce::Font(11.0f, juce::Font::bold));

    // Text shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawText(clipData.name,
               textBounds.translated(1, 1),
               juce::Justification::centredLeft,
               true);

    // Text
    g.setColour(Theme::Colours::text());
    g.drawText(clipData.name,
               textBounds,
               juce::Justification::centredLeft,
               true);

    // Selection or hover glow
    if (selected)
    {
        DrawingHelpers::drawGlow(g, bounds.reduced(1), Theme::Colours::accent(),
                                  Theme::Glow::radiusMedium, Theme::Glow::intensityNormal);

        g.setColour(Theme::Colours::accent());
        g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadiusSm, 2.0f);
    }
    else if (hovered)
    {
        // Subtle hover glow
        DrawingHelpers::drawGlow(g, bounds.reduced(1), clipData.colour.brighter(0.3f),
                                  Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle);

        g.setColour(clipData.colour.brighter(0.2f));
        g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadiusSm, 1.5f);
    }
    else
    {
        // Subtle border
        g.setColour(clipData.colour.darker(0.4f));
        g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadiusSm, 1.0f);
    }
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

    // Update waveform gradient colors based on clip color
    auto topColour = Theme::TrackColours::getWaveformTop(clip.colour);
    auto bottomColour = Theme::TrackColours::getWaveformBottom(clip.colour);
    waveformDisplay.setGradientColours(topColour, bottomColour);
    waveformDisplay.setBackgroundColour(clip.colour.darker(0.6f).withAlpha(0.3f));

    // Set tooltip with name and duration
    juce::String tooltip = clip.name + " (" + juce::String(clip.duration, 1) + "s)";
    setTooltip(tooltip);

    repaint();
}

void ClipComponent::setThumbnail(juce::AudioThumbnail* thumb)
{
    waveformDisplay.setThumbnail(thumb);

    // Set gradient colors based on clip color using TrackColours helpers
    auto topColour = Theme::TrackColours::getWaveformTop(clipData.colour);
    auto bottomColour = Theme::TrackColours::getWaveformBottom(clipData.colour);
    waveformDisplay.setGradientColours(topColour, bottomColour);
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

void ClipComponent::mouseEnter(const juce::MouseEvent& /*e*/)
{
    hovered = true;
    repaint();
}

void ClipComponent::mouseExit(const juce::MouseEvent& /*e*/)
{
    hovered = false;
    repaint();
}
