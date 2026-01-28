#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>

class MeterComponent : public juce::Component,
                       public juce::Timer
{
public:
    MeterComponent();
    ~MeterComponent() override;

    void paint(juce::Graphics& g) override;

    // Set current level (0.0 to 1.0+)
    void setLevel(float level);

    // Stereo mode
    void setStereo(bool stereo);
    void setLevels(float left, float right);

    // Timer for decay
    void timerCallback() override;

    // Reset peak hold
    void resetPeak();

private:
    void drawMeter(juce::Graphics& g, juce::Rectangle<float> bounds, float level, float peak);
    float linearToDecibels(float linear) const;
    float decibelsToY(float db, float height) const;

    std::atomic<float> leftLevel { 0.0f };
    std::atomic<float> rightLevel { 0.0f };
    std::atomic<float> leftPeak { 0.0f };
    std::atomic<float> rightPeak { 0.0f };

    float displayLeft = 0.0f;
    float displayRight = 0.0f;
    float peakHoldLeft = 0.0f;
    float peakHoldRight = 0.0f;

    bool stereoMode = true;

    // Decay rate
    static constexpr float decayRate = 0.92f;
    static constexpr float peakHoldTime = 1.5f; // seconds

    int peakHoldCounterLeft = 0;
    int peakHoldCounterRight = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterComponent)
};
