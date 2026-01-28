#include "ChannelStrip.h"

ChannelStrip::ChannelStrip()
{
    // Volume fader
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setRange(0.0, 2.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    volumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff64b5f6));
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
    panSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff9c27b0));
    panSlider.onValueChange = [this]()
    {
        if (onPanChanged)
            onPanChanged(trackId, static_cast<float>(panSlider.getValue()));
    };
    addAndMakeVisible(panSlider);

    // Mute button
    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff424242));
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this]()
    {
        muted = muteButton.getToggleState();
        if (onMuteChanged)
            onMuteChanged(trackId, muted);
    };
    addAndMakeVisible(muteButton);

    // Solo button
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff424242));
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    soloButton.setClickingTogglesState(true);
    soloButton.onClick = [this]()
    {
        solo = soloButton.getToggleState();
        if (onSoloChanged)
            onSoloChanged(trackId, solo);
    };
    addAndMakeVisible(soloButton);

    // Name label
    nameLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
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
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Color bar at top
    auto colorBar = getLocalBounds().removeFromTop(4);
    g.setColour(trackColour);
    g.fillRect(colorBar);

    // Border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRect(getLocalBounds());
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
