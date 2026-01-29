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
    void tempoChanged(double newBpm) override;

    // Tempo control
    void setTempo(double bpm);
    double getTempo() const { return tempoSlider.getValue(); }

    // Snap control
    void setSnapValue(int snapIndex);  // 0=Off, 1=Bar, 2=Beat, 3=Half, 4=Quarter, 5=Eighth, 6=Sixteenth
    int getSnapValue() const;

    // Callbacks
    std::function<void(double)> onTempoChanged;
    std::function<void(int)> onSnapValueChanged;  // Called when snap selector changes

    // Time signature for beat display
    void setTimeSignature(int numerator, int denominator);

    // Timer for position updates
    void timerCallback() override;

private:
    void updateButtonStates();
    void updatePositionDisplay();
    juce::String formatTime(double seconds) const;
    juce::String formatTimeAsBeats(double seconds) const;

    // Display mode
    bool showBeats = false;  // Toggle between MM:SS and Bar:Beat:Tick
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;

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

    // Snap selector
    juce::Label snapLabel { {}, "Snap:" };
    juce::ComboBox snapCombo;

    // Position display mode toggle
    juce::TextButton displayModeButton { "Time" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
