#pragma once

#include "../EffectSlot.h"
#include "Utils/Tweens.h"

/**
 * LFO-based tremolo effect with 30+ easing curves.
 * Ported from legacy XAudio2 implementation.
 */
class TremoloEffect : public EffectSlot
{
public:
    enum Parameter
    {
        Rate = 0,       // 0.1 to 20 Hz
        Depth,          // 0.0 to 1.0
        Shape,          // Easing type (0-30)
        NumParameters
    };

    TremoloEffect();
    ~TremoloEffect() override = default;

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

    juce::String getName() const override { return "Tremolo"; }
    juce::String getDescription() const override { return "LFO amplitude modulation"; }

    // Direct setters
    void setRate(float hz);
    void setDepth(float depth);
    void setShape(Tweening::EasingType type);
    void setShapeIndex(int index);

    // Get shape name for UI
    static juce::String getShapeName(int index);
    static int getNumShapes() { return 31; }

private:
    // LFO parameters
    float rate = 4.0f;          // Hz
    float depth = 0.5f;         // 0-1
    Tweening::EasingType shape = Tweening::EasingType::EaseInOutSine;

    // LFO state
    float phase = 0.0f;
    float phaseIncrement = 0.0f;

    double sampleRate = 44100.0;

    void updatePhaseIncrement();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloEffect)
};
