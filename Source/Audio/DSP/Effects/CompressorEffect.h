#pragma once

#include "../EffectSlot.h"

/**
 * Dynamics compressor with threshold, ratio, attack, release, and makeup gain.
 * Uses JUCE dsp::Compressor for processing.
 */
class CompressorEffect : public EffectSlot
{
public:
    enum Parameter
    {
        Threshold = 0,  // -60 to 0 dB
        Ratio,          // 1:1 to 20:1
        Attack,         // 0.1 to 100 ms
        Release,        // 10 to 1000 ms
        MakeupGain,     // 0 to 24 dB
        NumParameters
    };

    CompressorEffect();
    ~CompressorEffect() override = default;

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

    juce::String getName() const override { return "Compressor"; }
    juce::String getDescription() const override { return "Dynamics compressor"; }

    // Direct setters
    void setThreshold(float thresholdDb);
    void setRatio(float ratio);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setMakeupGain(float gainDb);

    // Gain reduction metering (for UI)
    float getGainReduction() const { return gainReduction; }

private:
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float> makeup;

    float threshold = -20.0f;
    float ratio = 4.0f;
    float attack = 10.0f;
    float release = 100.0f;
    float makeupGain = 0.0f;

    // For metering
    std::atomic<float> gainReduction { 0.0f };

    void updateCompressorParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorEffect)
};
