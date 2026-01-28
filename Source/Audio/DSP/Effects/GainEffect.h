#pragma once

#include "../EffectSlot.h"

/**
 * Simple gain effect with volume and pan controls.
 * Pan uses constant power panning for natural stereo image.
 */
class GainEffect : public EffectSlot
{
public:
    enum Parameter
    {
        Volume = 0,  // 0.0 to 2.0 (linear), 1.0 = unity
        Pan          // -1.0 (left) to +1.0 (right), 0.0 = center
    };

    GainEffect();
    ~GainEffect() override = default;

    // EffectSlot interface
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    void setParameter(int index, float value) override;
    float getParameter(int index) const override;
    juce::String getParameterName(int index) const override;
    int getNumParameters() const override { return 2; }

    float getParameterMin(int index) const override;
    float getParameterMax(int index) const override;
    float getParameterDefault(int index) const override;

    juce::String getName() const override { return "Gain"; }
    juce::String getDescription() const override { return "Volume and pan control"; }

    // Direct setters for convenience
    void setVolume(float newVolume);
    void setPan(float newPan);
    float getVolume() const { return volume; }
    float getPan() const { return pan; }

private:
    float volume = 1.0f;
    float pan = 0.0f;

    // Smoothed values to avoid clicks
    juce::SmoothedValue<float> smoothedVolume;
    juce::SmoothedValue<float> smoothedLeftGain;
    juce::SmoothedValue<float> smoothedRightGain;

    void updatePanGains();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainEffect)
};
