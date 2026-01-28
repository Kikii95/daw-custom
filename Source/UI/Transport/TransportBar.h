#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Audio/TransportController.h"

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

    // Position display
    juce::Label positionLabel;
    juce::Label durationLabel;

    // Tempo
    juce::Label tempoLabel { {}, "BPM:" };
    juce::Slider tempoSlider;

    // Colors
    juce::Colour bgColour { 0xff2a2a2a };
    juce::Colour playingColour { 0xff4caf50 };
    juce::Colour pausedColour { 0xffffc107 };
    juce::Colour stoppedColour { 0xff607d8b };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
