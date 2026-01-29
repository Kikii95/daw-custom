#include "TransportBar.h"

TransportBar::TransportBar()
{
    // Play button with icon
    playButton.setButtonText(juce::CharPointer_UTF8("\xe2\x96\xb6"));  // ▶
    playButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::success());
    playButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    playButton.setTooltip("Play (Space)");
    playButton.onClick = [this]()
    {
        if (transport != nullptr)
            transport->play();
    };
    addAndMakeVisible(playButton);

    // Pause button with icon
    pauseButton.setButtonText(juce::CharPointer_UTF8("\xe2\x8f\xb8"));  // ⏸
    pauseButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::warning());
    pauseButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    pauseButton.setTooltip("Pause");
    pauseButton.onClick = [this]()
    {
        if (transport != nullptr)
            transport->pause();
    };
    addAndMakeVisible(pauseButton);

    // Stop button with icon
    stopButton.setButtonText(juce::CharPointer_UTF8("\xe2\x8f\xb9"));  // ⏹
    stopButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::textMuted());
    stopButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    stopButton.setTooltip("Stop and reset (Enter)");
    stopButton.onClick = [this]()
    {
        if (transport != nullptr)
            transport->stop();
    };
    addAndMakeVisible(stopButton);

    // Position display
    positionLabel.setFont(juce::Font("Monospace", 18.0f, juce::Font::bold));
    positionLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    positionLabel.setJustificationType(juce::Justification::centred);
    positionLabel.setText("00:00.000", juce::dontSendNotification);
    addAndMakeVisible(positionLabel);

    // Duration display
    durationLabel.setFont(juce::Font("Monospace", 12.0f, juce::Font::plain));
    durationLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    durationLabel.setJustificationType(juce::Justification::centred);
    durationLabel.setText("/ 00:00.000", juce::dontSendNotification);
    addAndMakeVisible(durationLabel);

    // Tempo
    tempoLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    addAndMakeVisible(tempoLabel);

    tempoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider.setRange(20.0, 300.0, 0.1);
    tempoSlider.setValue(120.0);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    tempoSlider.setTooltip("Tempo: 20-300 BPM");
    tempoSlider.onValueChange = [this]()
    {
        if (transport != nullptr)
            transport->setTempo(tempoSlider.getValue());
    };
    addAndMakeVisible(tempoSlider);

    // Start timer for position updates
    startTimerHz(30);
}

TransportBar::~TransportBar()
{
    stopTimer();

    if (transport != nullptr)
        transport->removeListener(this);
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds().reduced(8);

    // Transport buttons on the left
    auto buttonArea = bounds.removeFromLeft(200);
    auto buttonWidth = buttonArea.getWidth() / 3;

    playButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    pauseButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    stopButton.setBounds(buttonArea.reduced(2));

    // Tempo on the right
    auto tempoArea = bounds.removeFromRight(200);
    tempoLabel.setBounds(tempoArea.removeFromLeft(40));
    tempoSlider.setBounds(tempoArea);

    // Position in center
    auto posArea = bounds.reduced(20, 0);
    positionLabel.setBounds(posArea.removeFromTop(posArea.getHeight() / 2 + 5));
    durationLabel.setBounds(posArea);
}

void TransportBar::paint(juce::Graphics& g)
{
    auto bgColour = Theme::colour(Theme::bgSlot);
    g.fillAll(bgColour);

    // Subtle gradient
    juce::ColourGradient gradient(bgColour.brighter(0.05f), 0, 0,
                                   bgColour.darker(0.05f), 0, static_cast<float>(getHeight()),
                                   false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    // Bottom border
    g.setColour(Theme::colour(Theme::bgDark));
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TransportBar::setTransportController(TransportController* controller)
{
    if (transport != nullptr)
        transport->removeListener(this);

    transport = controller;

    if (transport != nullptr)
    {
        transport->addListener(this);
        tempoSlider.setValue(transport->getTempo(), juce::dontSendNotification);
        updateButtonStates();
        updatePositionDisplay();
    }
}

void TransportBar::transportStateChanged(TransportController::State /*newState*/)
{
    juce::MessageManager::callAsync([this]()
    {
        updateButtonStates();
    });
}

void TransportBar::transportPositionChanged(double /*newPosition*/)
{
    // Position updates handled by timer for smoother display
}

void TransportBar::timerCallback()
{
    updatePositionDisplay();
}

void TransportBar::updateButtonStates()
{
    if (transport == nullptr)
        return;

    bool playing = transport->isPlaying();
    bool paused = transport->isPaused();

    playButton.setEnabled(!playing);
    pauseButton.setEnabled(playing);
    stopButton.setEnabled(playing || paused);
}

void TransportBar::updatePositionDisplay()
{
    if (transport == nullptr)
        return;

    positionLabel.setText(formatTime(transport->getPosition()), juce::dontSendNotification);
    durationLabel.setText("/ " + formatTime(transport->getDuration()), juce::dontSendNotification);
}

juce::String TransportBar::formatTime(double seconds) const
{
    int minutes = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int millis = static_cast<int>((seconds - std::floor(seconds)) * 1000);

    return juce::String::formatted("%02d:%02d.%03d", minutes, secs, millis);
}
