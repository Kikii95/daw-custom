#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class WaveformDisplay : public juce::Component,
                        public juce::ChangeListener
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;

    // Set the thumbnail to display
    void setThumbnail(juce::AudioThumbnail* thumb);

    // Set visible range (in seconds)
    void setVisibleRange(double startTime, double endTime);

    // Colors
    void setWaveformColour(juce::Colour colour) { waveformColour = colour; }
    void setBackgroundColour(juce::Colour colour) { backgroundColour = colour; }

    // Gradient mode
    void setUseGradient(bool use) { useGradient = use; repaint(); }
    void setGradientColours(juce::Colour top, juce::Colour bottom);

    // ChangeListener for thumbnail updates
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    void drawGradientWaveform(juce::Graphics& g, juce::Rectangle<float> bounds);

    juce::AudioThumbnail* thumbnail = nullptr;

    double visibleStart = 0.0;
    double visibleEnd = 10.0;

    juce::Colour waveformColour { 0xff64b5f6 };
    juce::Colour backgroundColour { 0xff2d2d2d };

    // Gradient mode
    bool useGradient = true;
    juce::Colour gradientTopColour { 0xff64b5f6 };
    juce::Colour gradientBottomColour { 0xff1565c0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
