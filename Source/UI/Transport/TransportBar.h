#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Audio/TransportController.h"
#include "UI/Theme/AppTheme.h"

class TransportBar : public juce::Component,
                     public juce::Timer,
                     public TransportController::Listener
{
public:
    TransportBar();
    ~TransportBar() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    // Bind to transport controller
    void setTransportController(TransportController* controller);

    // TransportController::Listener
    void transportStateChanged(TransportController::State newState) override;
    void transportPositionChanged(double newPosition) override;
    void transportLoopChanged(bool enabled, double start, double end) override;

    // Timer for position updates
    void timerCallback() override;

private:
    void updateButtonStates();
    void updatePositionDisplay();
    juce::String formatTime(double seconds) const;

    TransportController* transport = nullptr;

    // Buttons
    juce::TextButton playButton { "Play" };
    juce::TextButton pauseButton { "Pause" };
    juce::TextButton stopButton { "Stop" };
    juce::ToggleButton loopButton { "Loop" };
    juce::ToggleButton reverseButton { "Rev" };

    // Position display
    juce::Label positionLabel;
    juce::Label durationLabel;

    // Tempo
    juce::Label tempoLabel { {}, "BPM:" };
    juce::Slider tempoSlider;

    // Speed control
    juce::Label speedLabel { {}, "Speed:" };
    juce::Slider speedSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
