#pragma once

#include "../EffectSlot.h"
#include "Oscillators.h"
#include <juce_dsp/juce_dsp.h>

/**
 * Basic subtractive synthesizer combining:
 * - Dual oscillators (with detune)
 * - ADSR amplitude envelope
 * - Resonant lowpass filter with envelope modulation
 *
 * Signal flow:
 *   Osc1 + Osc2(detuned) → Filter(cutoff modulated by env) → AmpEnv → Output
 */
class BasicSynth : public EffectSlot
{
public:
    enum Parameter
    {
        // Oscillator
        Frequency = 0,    // 20-2000 Hz (note frequency)
        Waveform,         // 0-3 (Sine, Square, Saw, Triangle)
        Osc2Detune,       // -24 to +24 cents
        Osc2Mix,          // 0-1 (mix of osc2 with osc1)

        // Filter
        FilterCutoff,     // 20-20000 Hz
        FilterReso,       // 0-1
        FilterEnvAmt,     // 0-1 (how much envelope affects filter)

        // Amp Envelope
        Attack,           // 1-5000 ms
        Decay,            // 1-5000 ms
        Sustain,          // 0-1
        Release,          // 1-5000 ms

        // Output
        Level,            // 0-1

        NumParams
    };

    BasicSynth();
    ~BasicSynth() override = default;

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
    juce::String getName() const override { return "Synth"; }
    juce::String getDescription() const override { return "Basic subtractive synthesizer"; }

    // Trigger controls
    void trigger();
    void release();
    bool isPlaying() const { return ampEnvValue > 0.001f || state != EnvIdle; }

private:
    enum EnvState { EnvIdle, EnvAttack, EnvDecay, EnvSustain, EnvRelease };

    // Oscillator state
    float phase1 = 0.0f;
    float phase2 = 0.0f;

    // Envelope state
    EnvState state = EnvIdle;
    float ampEnvValue = 0.0f;
    float filterEnvValue = 0.0f;

    // Filter state
    juce::dsp::StateVariableTPTFilter<float> filterL;
    juce::dsp::StateVariableTPTFilter<float> filterR;

    // Parameters - Oscillator
    float frequency = 220.0f;
    Synthesis::Waveform waveform = Synthesis::Waveform::Sawtooth;
    float osc2Detune = 5.0f;      // Cents
    float osc2Mix = 0.5f;

    // Parameters - Filter
    float filterCutoff = 2000.0f;
    float filterReso = 0.3f;
    float filterEnvAmt = 0.5f;

    // Parameters - Envelope (ms)
    float attackMs = 10.0f;
    float decayMs = 200.0f;
    float sustainLevel = 0.7f;
    float releaseMs = 300.0f;

    // Output
    float level = 0.7f;

    // Envelope increments
    float attackInc = 0.0f;
    float decayInc = 0.0f;
    float releaseInc = 0.0f;

    // Audio
    double sampleRate = 44100.0;
    juce::Random noiseGen;
    bool hasTriggered = false;

    // Smoothing
    juce::SmoothedValue<float> smoothedFreq;
    juce::SmoothedValue<float> smoothedCutoff;

    // Helpers
    void updateEnvelopeIncrements();
    float processEnvelope();
    float generateOscillator(float& phase, float freq);
    void updateFilter(float envValue);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicSynth)
};
