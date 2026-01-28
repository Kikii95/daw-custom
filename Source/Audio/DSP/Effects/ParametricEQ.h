#pragma once

#include "../EffectSlot.h"
#include <array>

/**
 * 3-band parametric EQ with Low Shelf, Mid Peak, High Shelf.
 * Uses JUCE IIR filters for DSP processing.
 */
class ParametricEQ : public EffectSlot
{
public:
    enum Parameter
    {
        LowFreq = 0,    // 20 - 500 Hz
        LowGain,        // -24 to +24 dB
        MidFreq,        // 200 - 8000 Hz
        MidGain,        // -24 to +24 dB
        MidQ,           // 0.1 to 10.0
        HighFreq,       // 2000 - 20000 Hz
        HighGain,       // -24 to +24 dB
        NumParameters
    };

    ParametricEQ();
    ~ParametricEQ() override = default;

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

    juce::String getName() const override { return "Parametric EQ"; }
    juce::String getDescription() const override { return "3-band parametric equalizer"; }

    // Direct setters
    void setLowShelf(float freq, float gainDb);
    void setMidPeak(float freq, float gainDb, float q);
    void setHighShelf(float freq, float gainDb);

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    // Stereo filter pairs (L/R)
    std::array<Filter, 2> lowFilters;
    std::array<Filter, 2> midFilters;
    std::array<Filter, 2> highFilters;

    // Parameters
    float lowFreq = 200.0f;
    float lowGain = 0.0f;
    float midFreq = 1000.0f;
    float midGain = 0.0f;
    float midQ = 1.0f;
    float highFreq = 5000.0f;
    float highGain = 0.0f;

    void updateLowCoefficients();
    void updateMidCoefficients();
    void updateHighCoefficients();
    void updateAllCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQ)
};
