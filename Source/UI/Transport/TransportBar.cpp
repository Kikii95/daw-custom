#include "TransportBar.h"
#include "UI/Theme/DrawingHelpers.h"

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

    // Loop toggle button
    loopButton.setButtonText("L");
    loopButton.setColour(juce::ToggleButton::textColourId, Theme::Colours::text());
    loopButton.setColour(juce::ToggleButton::tickColourId, Theme::Colours::accent());
    loopButton.setTooltip("Toggle loop mode (Alt+click ruler to set loop)");
    loopButton.onClick = [this]()
    {
        if (transport != nullptr)
            transport->setLoopEnabled(loopButton.getToggleState());
    };
    addAndMakeVisible(loopButton);

    // Reverse toggle button
    reverseButton.setButtonText(juce::CharPointer_UTF8("\xe2\x8f\xaa"));  // ⏪
    reverseButton.setColour(juce::ToggleButton::textColourId, Theme::Colours::text());
    reverseButton.setColour(juce::ToggleButton::tickColourId, Theme::Colours::accent());
    reverseButton.setTooltip("Toggle reverse playback");
    reverseButton.onClick = [this]()
    {
        if (transport != nullptr)
            transport->setReversed(reverseButton.getToggleState());
    };
    addAndMakeVisible(reverseButton);

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
        double bpm = tempoSlider.getValue();
        if (transport != nullptr)
            transport->setTempo(bpm);
        if (onTempoChanged)
            onTempoChanged(bpm);
    };
    addAndMakeVisible(tempoSlider);

    // Speed control
    speedLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    addAndMakeVisible(speedLabel);

    speedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    speedSlider.setRange(0.25, 4.0, 0.05);
    speedSlider.setValue(1.0);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    speedSlider.setTooltip("Playback speed: 0.25x - 4x");
    speedSlider.onValueChange = [this]()
    {
        if (transport != nullptr)
            transport->setPlaybackSpeed(speedSlider.getValue());
    };
    addAndMakeVisible(speedSlider);

    // Snap selector
    snapLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    addAndMakeVisible(snapLabel);

    snapCombo.addItem("Off", 1);
    snapCombo.addItem("Bar", 2);
    snapCombo.addItem("Beat", 3);
    snapCombo.addItem("1/2", 4);
    snapCombo.addItem("1/4", 5);
    snapCombo.addItem("1/8", 6);
    snapCombo.addItem("1/16", 7);
    snapCombo.setSelectedId(7);  // Default: 1/16
    snapCombo.setTooltip("Snap grid resolution (G to toggle on/off)");
    snapCombo.onChange = [this]()
    {
        int snapIndex = snapCombo.getSelectedId() - 1;  // Convert to 0-based
        if (onSnapValueChanged)
            onSnapValueChanged(snapIndex);
    };
    addAndMakeVisible(snapCombo);

    // Position display mode toggle
    displayModeButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgSlot));
    displayModeButton.setColour(juce::TextButton::textColourOffId, Theme::Colours::textMuted());
    displayModeButton.setTooltip("Toggle between Time (MM:SS) and Beats (Bar:Beat:Tick)");
    displayModeButton.onClick = [this]()
    {
        showBeats = !showBeats;
        displayModeButton.setButtonText(showBeats ? "Beats" : "Time");
        updatePositionDisplay();
    };
    addAndMakeVisible(displayModeButton);

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

    // Transport buttons on the left (play, pause, stop, loop, reverse)
    auto buttonArea = bounds.removeFromLeft(300);
    auto buttonWidth = buttonArea.getWidth() / 5;

    playButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    pauseButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    stopButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    loopButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    reverseButton.setBounds(buttonArea.reduced(2));

    // Speed control on the right
    auto speedArea = bounds.removeFromRight(150);
    speedLabel.setBounds(speedArea.removeFromLeft(45));
    speedSlider.setBounds(speedArea);

    // Snap selector
    auto snapArea = bounds.removeFromRight(110);
    snapLabel.setBounds(snapArea.removeFromLeft(35));
    snapCombo.setBounds(snapArea);

    // Tempo
    auto tempoArea = bounds.removeFromRight(180);
    tempoLabel.setBounds(tempoArea.removeFromLeft(35));
    tempoSlider.setBounds(tempoArea);

    // Display mode toggle (small button)
    auto modeArea = bounds.removeFromRight(50);
    displayModeButton.setBounds(modeArea.reduced(2));

    // Position in center
    auto posArea = bounds.reduced(20, 0);
    positionLabel.setBounds(posArea.removeFromTop(posArea.getHeight() / 2 + 5));
    durationLabel.setBounds(posArea);
}

void TransportBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto bgColour = Theme::colour(Theme::bgSlot);

    // Background gradient (deeper)
    juce::ColourGradient gradient(bgColour.brighter(0.08f), 0, 0,
                                   bgColour.darker(0.1f), 0, bounds.getHeight(),
                                   false);
    g.setGradientFill(gradient);
    g.fillRect(bounds);

    // Inner shadow at top (inset effect)
    DrawingHelpers::drawInnerShadow(g, bounds, 3.0f,
                                     juce::Colours::black.withAlpha(0.3f), 0.0f);

    // Time display area background with glow
    auto positionBounds = juce::Rectangle<float>(
        bounds.getCentreX() - 80, 8,
        160, bounds.getHeight() - 16
    );

    // Subtle glow around time display
    DrawingHelpers::drawGlow(g, positionBounds, Theme::Colours::accent(),
                              Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle * 0.5f);

    // Time display background
    g.setColour(Theme::colour(Theme::bgDark).withAlpha(0.5f));
    g.fillRoundedRectangle(positionBounds, Theme::cornerRadiusSm);

    // Time display border
    g.setColour(Theme::colour(Theme::border).withAlpha(0.5f));
    g.drawRoundedRectangle(positionBounds, Theme::cornerRadiusSm, 1.0f);

    // Top highlight line
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(1, 0, bounds.getWidth());

    // Bottom border (shadow line)
    g.setColour(Theme::colour(Theme::bgDark));
    g.fillRect(0.0f, bounds.getHeight() - 2, bounds.getWidth(), 2.0f);

    // Bottom accent line
    g.setColour(Theme::Colours::accent().withAlpha(0.2f));
    g.fillRect(0.0f, bounds.getHeight() - 1, bounds.getWidth(), 1.0f);
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
        speedSlider.setValue(transport->getPlaybackSpeed(), juce::dontSendNotification);
        reverseButton.setToggleState(transport->isReversed(), juce::dontSendNotification);
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

void TransportBar::transportLoopChanged(bool enabled, double /*start*/, double /*end*/)
{
    juce::MessageManager::callAsync([this, enabled]()
    {
        loopButton.setToggleState(enabled, juce::dontSendNotification);
    });
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
    bool stopped = transport->isStopped();
    double position = transport->getPosition();

    // Play: enabled when not playing (can restart from stopped or resume from paused)
    playButton.setEnabled(!playing);

    // Pause: enabled only when playing
    pauseButton.setEnabled(playing);

    // Stop: always enabled if there's something to reset (position > 0 or not stopped)
    // This allows explicit reset even after playback ends naturally
    stopButton.setEnabled(playing || paused || position > 0.0);
}

void TransportBar::updatePositionDisplay()
{
    if (transport == nullptr)
        return;

    if (showBeats)
    {
        positionLabel.setText(formatTimeAsBeats(transport->getPosition()), juce::dontSendNotification);
        durationLabel.setText("/ " + formatTimeAsBeats(transport->getDuration()), juce::dontSendNotification);
    }
    else
    {
        positionLabel.setText(formatTime(transport->getPosition()), juce::dontSendNotification);
        durationLabel.setText("/ " + formatTime(transport->getDuration()), juce::dontSendNotification);
    }
}

juce::String TransportBar::formatTime(double seconds) const
{
    int minutes = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int millis = static_cast<int>((seconds - std::floor(seconds)) * 1000);

    return juce::String::formatted("%02d:%02d.%03d", minutes, secs, millis);
}

juce::String TransportBar::formatTimeAsBeats(double seconds) const
{
    double bpm = tempoSlider.getValue();
    if (bpm <= 0.0)
        bpm = 120.0;

    double beatDuration = 60.0 / bpm;
    double barDuration = beatDuration * timeSignatureNum;

    // Calculate bar (1-based)
    int bars = static_cast<int>(seconds / barDuration) + 1;

    // Calculate beat within bar (1-based)
    double remaining = std::fmod(seconds, barDuration);
    int beats = static_cast<int>(remaining / beatDuration) + 1;

    // Calculate ticks within beat (0-959, standard 960 PPQ)
    double beatRemainder = std::fmod(remaining, beatDuration);
    int ticks = static_cast<int>((beatRemainder / beatDuration) * 960);

    return juce::String::formatted("%d:%d:%03d", bars, beats, ticks);
}

void TransportBar::tempoChanged(double newBpm)
{
    juce::MessageManager::callAsync([this, newBpm]()
    {
        tempoSlider.setValue(newBpm, juce::dontSendNotification);
    });
}

void TransportBar::setTempo(double bpm)
{
    tempoSlider.setValue(bpm, juce::dontSendNotification);
}

void TransportBar::setSnapValue(int snapIndex)
{
    snapCombo.setSelectedId(snapIndex + 1, juce::dontSendNotification);
}

int TransportBar::getSnapValue() const
{
    return snapCombo.getSelectedId() - 1;
}

void TransportBar::setTimeSignature(int numerator, int denominator)
{
    timeSignatureNum = numerator;
    timeSignatureDenom = denominator;
    updatePositionDisplay();
}
