#pragma once

#include "../EffectSlot.h"

/**
 * Reverb effect using JUCE's Freeverb implementation.
 * Controls: Room Size, Damping, Width, Wet/Dry Mix.
 */
class ReverbEffect : public EffectSlot
{
public:
    enum Parameter
    {
        RoomSize = 0,   // 0.0 to 1.0
        Damping,        // 0.0 to 1.0
        Width,          // 0.0 to 1.0 (stereo width)
        WetLevel,       // 0.0 to 1.0
        DryLevel,       // 0.0 to 1.0
        NumParameters
    };

    ReverbEffect();
    ~ReverbEffect() override = default;

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

    juce::String getName() const override { return "Reverb"; }
    juce::String getDescription() const override { return "Freeverb reverb"; }

    // Direct setters
    void setRoomSize(float size);
    void setDamping(float damping);
    void setWidth(float width);
    void setWetLevel(float wet);
    void setDryLevel(float dry);

    // Preset helpers
    void setSmallRoom();
    void setLargeHall();
    void setPlate();

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters params;

    void updateReverbParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbEffect)
};
