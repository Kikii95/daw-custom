#pragma once

#include "../EffectSlot.h"

/**
 * Stereo delay effect with feedback.
 * Ported from legacy XAudio2 implementation, using JUCE DelayLine.
 */
class DelayEffect : public EffectSlot
{
public:
    enum Parameter
    {
        DelayTime = 0,  // 0 to 2000 ms
        Feedback,       // 0.0 to 0.95
        Mix,            // 0.0 (dry) to 1.0 (wet)
        NumParameters
    };

    DelayEffect();
    ~DelayEffect() override = default;

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

    juce::String getName() const override { return "Delay"; }
    juce::String getDescription() const override { return "Stereo delay with feedback"; }

    // Direct setters
    void setDelayTime(float timeMs);
    void setFeedback(float fb);
    void setDryWetMix(float mixValue);

    // Sync to tempo (optional)
    void syncToTempo(double bpm, float noteValue);

private:
    static constexpr int MAX_DELAY_SAMPLES = 2 * 48000;  // 2 seconds @ 48kHz

    // Stereo delay lines
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineL { MAX_DELAY_SAMPLES };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineR { MAX_DELAY_SAMPLES };

    // Parameters
    float delayTimeMs = 250.0f;
    float feedback = 0.4f;
    float dryWetMix = 0.5f;

    // Smoothed delay time to avoid clicks
    juce::SmoothedValue<float> smoothedDelayL;
    juce::SmoothedValue<float> smoothedDelayR;

    double currentSampleRate = 44100.0;

    void updateDelayTime();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayEffect)
};
