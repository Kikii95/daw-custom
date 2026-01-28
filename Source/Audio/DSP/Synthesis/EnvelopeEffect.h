#pragma once

#include "../EffectSlot.h"
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * ADSR Envelope effect that modulates amplitude.
 * Can be triggered/released for note-like behavior.
 * Auto-triggers on first process call if not manually triggered.
 */
class EnvelopeEffect : public EffectSlot
{
public:
    enum State
    {
        Idle = 0,
        Attack,
        Decay,
        Sustain,
        Release
    };

    enum Parameter
    {
        AttackMs = 0,     // 1-5000 ms
        DecayMs,          // 1-5000 ms
        SustainLevel,     // 0-1
        ReleaseMs,        // 1-5000 ms
        AutoTrigger,      // 0 or 1 (auto-trigger on process start)
        NumParams
    };

    EnvelopeEffect();
    ~EnvelopeEffect() override = default;

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
    juce::String getName() const override { return "Envelope"; }
    juce::String getDescription() const override { return "ADSR amplitude envelope"; }

    // Trigger controls
    void trigger();               // Start attack phase
    void release();               // Start release phase
    void forceIdle();             // Immediately go to idle

    // State queries
    State getState() const { return state; }
    float getCurrentValue() const { return envelopeValue; }
    bool isActive() const { return state != Idle; }

private:
    // Envelope state
    State state = Idle;
    float envelopeValue = 0.0f;
    float stateProgress = 0.0f;    // Progress within current state (0-1)

    // ADSR parameters (ms)
    float attackMs = 10.0f;
    float decayMs = 100.0f;
    float sustainLevel = 0.7f;
    float releaseMs = 200.0f;
    bool autoTrigger = true;

    // Precomputed per-sample increments
    float attackIncrement = 0.0f;
    float decayIncrement = 0.0f;
    float releaseIncrement = 0.0f;

    // Sample rate
    double sampleRate = 44100.0;

    // Helpers
    void updateIncrements();
    float processEnvelope();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeEffect)
};
