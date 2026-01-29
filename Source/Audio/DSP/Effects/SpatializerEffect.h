#pragma once

#include "../EffectSlot.h"

/**
 * 3D Spatializer effect using azimuth/elevation/distance for stereo panning.
 * Cross-platform JUCE implementation (replaces legacy X3DAudio).
 */
class SpatializerEffect : public EffectSlot
{
public:
    enum Parameter
    {
        Azimuth = 0,    // -180 to +180 degrees (left/right)
        Elevation,      // -90 to +90 degrees (up/down)
        Distance,       // 0.1 to 100 meters
        NumParameters
    };

    SpatializerEffect();
    ~SpatializerEffect() override = default;

    // EffectSlot interface
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    void setParameter(int index, float value) override;
    float getParameter(int index) const override;
    juce::String getParameterName(int index) const override;
    int getNumParameters() const override { return NumParameters; }

    float getParameterMin(int index) const override;
    float getParameterMax(int index) const override;
    float getParameterDefault(int index) const override;

    juce::String getName() const override { return "Spatializer 3D"; }
    juce::String getDescription() const override { return "3D spatial positioning"; }

    // Direct setters
    void setAzimuth(float degrees);
    void setElevation(float degrees);
    void setDistance(float meters);

private:
    // Parameters
    std::atomic<float> azimuth { 0.0f };      // -180 to +180
    std::atomic<float> elevation { 0.0f };    // -90 to +90
    std::atomic<float> distance { 1.0f };     // 0.1 to 100

    // Calculated gains
    float leftGain = 0.707f;
    float rightGain = 0.707f;
    float distanceAttenuation = 1.0f;

    // Smoothed gains for click-free changes
    juce::SmoothedValue<float> smoothedLeftGain;
    juce::SmoothedValue<float> smoothedRightGain;

    double currentSampleRate = 44100.0;

    void updateGains();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatializerEffect)
};
