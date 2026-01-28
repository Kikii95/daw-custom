#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
 * Base class for all audio effects in the DSP chain.
 * Inherits from juce::dsp::ProcessorBase for JUCE DSP compatibility.
 */
class EffectSlot : public juce::dsp::ProcessorBase
{
public:
    EffectSlot() = default;
    virtual ~EffectSlot() = default;

    // ProcessorBase interface
    void prepare(const juce::dsp::ProcessSpec& spec) override = 0;
    void reset() override = 0;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override = 0;

    // Parameter interface
    virtual void setParameter(int index, float value) = 0;
    virtual float getParameter(int index) const = 0;
    virtual juce::String getParameterName(int index) const = 0;
    virtual int getNumParameters() const = 0;

    // Parameter range helpers (optional override)
    virtual float getParameterMin(int /*index*/) const { return 0.0f; }
    virtual float getParameterMax(int /*index*/) const { return 1.0f; }
    virtual float getParameterDefault(int /*index*/) const { return 0.0f; }

    // Effect info
    virtual juce::String getName() const = 0;
    virtual juce::String getDescription() const { return {}; }

    // Bypass control
    void setBypass(bool shouldBypass) { bypassed = shouldBypass; }
    bool isBypassed() const { return bypassed; }

    // Wet/Dry mix (0.0 = fully dry, 1.0 = fully wet)
    void setMix(float newMix) { mix = juce::jlimit(0.0f, 1.0f, newMix); }
    float getMix() const { return mix; }

protected:
    bool bypassed = false;
    float mix = 1.0f;
    juce::dsp::ProcessSpec currentSpec { 0, 0, 0 };

    // Helper: apply wet/dry mix to output
    void applyMix(juce::AudioBuffer<float>& wetBuffer,
                  const juce::AudioBuffer<float>& dryBuffer)
    {
        if (mix >= 1.0f)
            return;

        for (int ch = 0; ch < wetBuffer.getNumChannels(); ++ch)
        {
            auto* wet = wetBuffer.getWritePointer(ch);
            auto* dry = dryBuffer.getReadPointer(ch);

            for (int i = 0; i < wetBuffer.getNumSamples(); ++i)
                wet[i] = dry[i] * (1.0f - mix) + wet[i] * mix;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectSlot)
};
