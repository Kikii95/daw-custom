#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>

/**
 * Real-time waveform oscilloscope display.
 * Uses a circular buffer to display incoming audio samples.
 */
class OscilloscopePanel : public juce::Component,
                          public juce::Timer
{
public:
    OscilloscopePanel();
    ~OscilloscopePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Timer for repainting
    void timerCallback() override;

    // Push samples from audio thread
    void pushSamples(const float* samples, int numSamples);
    void pushStereoSamples(const float* left, const float* right, int numSamples);

    // Display settings
    void setTimeWindow(float seconds) { timeWindow = seconds; }
    float getTimeWindow() const { return timeWindow; }

    void setGain(float g) { displayGain = juce::jlimit(0.1f, 10.0f, g); }
    float getGain() const { return displayGain; }

    void setShowStereo(bool show) { showStereo = show; }
    bool isShowingStereo() const { return showStereo; }

    void setFrozen(bool freeze) { frozen = freeze; }
    bool isFrozen() const { return frozen; }

private:
    static constexpr int BUFFER_SIZE = 4096;

    // Circular buffer for waveform data
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    std::atomic<int> writeIndex { 0 };

    // Display settings
    float timeWindow = 0.05f;       // 50ms default
    float displayGain = 1.0f;
    bool showStereo = true;
    bool frozen = false;

    // Colors
    juce::Colour leftColour { 0xff00ff88 };   // Green
    juce::Colour rightColour { 0xffff8800 };  // Orange
    juce::Colour gridColour { 0x33ffffff };

    void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawWaveform(juce::Graphics& g, juce::Rectangle<float> bounds,
                      const std::vector<float>& buffer, juce::Colour colour);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopePanel)
};
