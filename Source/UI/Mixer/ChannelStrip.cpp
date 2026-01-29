#include "ChannelStrip.h"
#include "UI/Theme/AppTheme.h"
#include "UI/Theme/DrawingHelpers.h"

ChannelStrip::ChannelStrip()
{
    // Volume fader
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setRange(0.0, 2.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    volumeSlider.setColour(juce::Slider::thumbColourId, Theme::colour(Theme::accentBlue));
    volumeSlider.setTooltip("Volume: 0% to 200%");
    volumeSlider.onValueChange = [this]()
    {
        if (onVolumeChanged)
            onVolumeChanged(trackId, static_cast<float>(volumeSlider.getValue()));
    };
    addAndMakeVisible(volumeSlider);

    // Pan knob
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(0.0);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.setColour(juce::Slider::thumbColourId, Theme::colour(Theme::accentPurple));
    panSlider.setTooltip("Pan: Left/Right balance");
    panSlider.onValueChange = [this]()
    {
        if (onPanChanged)
            onPanChanged(trackId, static_cast<float>(panSlider.getValue()));
    };
    addAndMakeVisible(panSlider);

    // Mute button
    muteButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgHover));
    muteButton.setColour(juce::TextButton::buttonOnColourId, Theme::Colours::error());
    muteButton.setClickingTogglesState(true);
    muteButton.setTooltip("Mute track (M)");
    muteButton.onClick = [this]()
    {
        muted = muteButton.getToggleState();
        if (onMuteChanged)
            onMuteChanged(trackId, muted);
    };
    addAndMakeVisible(muteButton);

    // Solo button
    soloButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgHover));
    soloButton.setColour(juce::TextButton::buttonOnColourId, Theme::colour(Theme::meterMid));
    soloButton.setClickingTogglesState(true);
    soloButton.setTooltip("Solo track (S)");
    soloButton.onClick = [this]()
    {
        solo = soloButton.getToggleState();
        if (onSoloChanged)
            onSoloChanged(trackId, solo);
    };
    addAndMakeVisible(soloButton);

    // Name label
    nameLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    nameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(nameLabel);

    // Meter
    addAndMakeVisible(meter);
}

ChannelStrip::~ChannelStrip()
{
}

void ChannelStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Drop shadow
    DrawingHelpers::drawShadow(g, bounds.reduced(2),
                                selected ? Theme::Shadows::md : Theme::Shadows::sm,
                                Theme::cornerRadius);

    // Determine background color based on state
    juce::Colour bgBase = Theme::colour(Theme::bgSlot);
    if (selected)
        bgBase = Theme::colour(Theme::bgSelected);
    else if (hovered)
        bgBase = Theme::colour(Theme::bgHover);

    // Background gradient
    auto bgGradient = juce::ColourGradient(
        bgBase.brighter(0.05f), bounds.getX(), bounds.getY(),
        bgBase.darker(0.03f), bounds.getX(), bounds.getBottom(),
        false);

    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds.reduced(2), Theme::cornerRadius);

    // Color bar at top with gradient and glow
    auto colorBar = bounds.removeFromTop(6).reduced(2, 0);
    colorBar = colorBar.withTrimmedTop(2);

    // Color bar glow (underneath)
    DrawingHelpers::drawGlow(g, colorBar, trackColour, 6.0f, 0.35f);

    // Color bar gradient
    auto colorGradient = juce::ColourGradient(
        trackColour.brighter(0.2f), colorBar.getX(), colorBar.getY(),
        trackColour.darker(0.1f), colorBar.getX(), colorBar.getBottom(),
        false);
    g.setGradientFill(colorGradient);
    g.fillRoundedRectangle(colorBar, 3.0f);

    // Selection glow
    if (selected)
    {
        DrawingHelpers::drawGlow(g, bounds.reduced(2), Theme::Colours::accent(),
                                  Theme::Glow::radiusMedium, Theme::Glow::intensitySubtle);

        g.setColour(Theme::Colours::accent());
        g.drawRoundedRectangle(bounds.reduced(2), Theme::cornerRadius, 2.0f);
    }
    else
    {
        // Subtle border
        g.setColour(Theme::colour(Theme::border));
        g.drawRoundedRectangle(bounds.reduced(2), Theme::cornerRadius, 1.0f);
    }
}

void ChannelStrip::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    bounds.removeFromTop(4);  // Color bar space

    // Name at top
    nameLabel.setBounds(bounds.removeFromTop(20));

    // Mute/Solo buttons
    auto buttonArea = bounds.removeFromTop(24);
    muteButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2).reduced(2));
    soloButton.setBounds(buttonArea.reduced(2));

    bounds.removeFromTop(4);  // Gap

    // Pan knob
    panSlider.setBounds(bounds.removeFromTop(50).reduced(10, 0));

    bounds.removeFromTop(4);  // Gap

    // Meter on the right, fader on the left
    auto faderMeterArea = bounds;
    meter.setBounds(faderMeterArea.removeFromRight(20));
    faderMeterArea.removeFromRight(4);
    volumeSlider.setBounds(faderMeterArea);
}

void ChannelStrip::setTrackData(const Track& track)
{
    trackId = track.id;
    trackName = track.name;
    trackColour = track.colour;

    nameLabel.setText(trackName, juce::dontSendNotification);
    volumeSlider.setValue(track.volume, juce::dontSendNotification);
    panSlider.setValue(track.pan, juce::dontSendNotification);
    muteButton.setToggleState(track.muted, juce::dontSendNotification);
    soloButton.setToggleState(track.solo, juce::dontSendNotification);

    muted = track.muted;
    solo = track.solo;

    repaint();
}

void ChannelStrip::setMeterLevels(float left, float right)
{
    meter.setLevels(left, right);
}

void ChannelStrip::setSelected(bool shouldBeSelected)
{
    if (selected != shouldBeSelected)
    {
        selected = shouldBeSelected;
        repaint();
    }
}

void ChannelStrip::mouseDown(const juce::MouseEvent& e)
{
    juce::Component::mouseDown(e);

    if (onSelected)
        onSelected(trackId);
}

void ChannelStrip::mouseEnter(const juce::MouseEvent& /*e*/)
{
    hovered = true;
    repaint();
}

void ChannelStrip::mouseExit(const juce::MouseEvent& /*e*/)
{
    hovered = false;
    repaint();
}
