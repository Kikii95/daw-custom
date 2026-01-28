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

    // ChangeListener for thumbnail updates
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    juce::AudioThumbnail* thumbnail = nullptr;

    double visibleStart = 0.0;
    double visibleEnd = 10.0;

    juce::Colour waveformColour { 0xff64b5f6 };
    juce::Colour backgroundColour { 0xff2d2d2d };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
