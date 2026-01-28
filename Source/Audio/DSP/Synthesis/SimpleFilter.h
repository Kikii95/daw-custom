#pragma once

#include "../EffectSlot.h"
#include <juce_dsp/juce_dsp.h>

/**
 * Simple resonant filter with LP, HP, and BP modes.
 * Uses JUCE StateVariableTPTFilter for quality and stability.
 */
class SimpleFilter : public EffectSlot
{
public:
    enum FilterMode
    {
        LowPass = 0,
        HighPass,
        BandPass
    };

    enum Parameter
    {
        Cutoff = 0,       // 20-20000 Hz
        Resonance,        // 0-1 (maps to Q 0.5-10)
        Mode,             // 0=LP, 1=HP, 2=BP
        EnvAmount,        // -1 to 1 (envelope modulation depth)
        NumParams
    };

    SimpleFilter();
    ~SimpleFilter() override = default;

    // EffectSlot interface
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    // Parameter interface
    void setParameter(int index, float value) override;
    float getParameter(int index) const override;
    juce::String getParameterName(int index) const override;
    int getNumParameters() const override { return NumParams; }

    // Parameter ranges
    float getParameterMin(int index) const override;
    float getParameterMax(int index) const override;
    float getParameterDefault(int index) const override;

    // Effect info
    juce::String getName() const override { return "Filter"; }
    juce::String getDescription() const override { return "Resonant LP/HP/BP filter"; }

    // Direct control (for modulation from other effects)
    void setCutoff(float hz) { cutoffHz = juce::jlimit(20.0f, 20000.0f, hz); updateFilter(); }
    void setResonance(float r) { resonance = juce::jlimit(0.0f, 1.0f, r); updateFilter(); }
    void setMode(FilterMode m) { mode = m; updateFilter(); }

    // Envelope modulation input (0-1 from external envelope)
    void setEnvelopeValue(float env) { envelopeValue = env; }

private:
    // Filter state
    juce::dsp::StateVariableTPTFilter<float> filterL;
    juce::dsp::StateVariableTPTFilter<float> filterR;

    // Parameters
    float cutoffHz = 1000.0f;
    float resonance = 0.0f;      // 0-1
    FilterMode mode = LowPass;
    float envAmount = 0.0f;      // -1 to 1

    // Envelope modulation
    float envelopeValue = 0.0f;

    // Smoothing
    juce::SmoothedValue<float> smoothedCutoff;

    // Audio
    double sampleRate = 44100.0;

    // Helpers
    void updateFilter();
    float calculateModulatedCutoff();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleFilter)
};
