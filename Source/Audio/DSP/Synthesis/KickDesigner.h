#pragma once

#include "../EffectSlot.h"
#include "Oscillators.h"
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * Kick drum synthesizer using sine + pitch envelope + noise + saturation.
 * Designed for electronic kicks (808, 909 style).
 *
 * Signal flow:
 *   Sine(freq * pitchEnv) → Mix with Noise → Saturation → AmpEnv → Output
 */
class KickDesigner : public EffectSlot
{
public:
    enum Parameter
    {
        BaseFreq = 0,     // Fundamental frequency (40-120 Hz)
        PitchAmount,      // Pitch envelope depth in semitones (0-48)
        PitchDecay,       // Pitch envelope decay time (5-500 ms)
        BodyDecay,        // Amplitude decay time (50-2000 ms)
        NoiseAmount,      // Noise mix (0-1)
        NoiseDecay,       // Noise decay time (1-100 ms)
        Saturation,       // Drive amount (0-1)
        Click,            // Initial click intensity (0-1)
        Level,            // Output level (0-1)
        NumParams
    };

    KickDesigner();
    ~KickDesigner() override = default;

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
    juce::String getName() const override { return "Kick"; }
    juce::String getDescription() const override { return "808-style kick drum synthesizer"; }

    // Trigger control
    void trigger();
    bool isPlaying() const { return ampEnvValue > 0.001f; }

private:
    // Oscillator state
    float phase = 0.0f;
    float currentFreq = 60.0f;

    // Envelope values (0-1)
    float pitchEnvValue = 0.0f;
    float ampEnvValue = 0.0f;
    float noiseEnvValue = 0.0f;
    float clickEnvValue = 0.0f;

    // Per-sample decay coefficients
    float pitchDecayCoeff = 0.999f;
    float ampDecayCoeff = 0.9999f;
    float noiseDecayCoeff = 0.99f;
    float clickDecayCoeff = 0.95f;

    // Parameters
    float baseFreq = 60.0f;       // Hz
    float pitchAmount = 36.0f;    // Semitones
    float pitchDecayMs = 50.0f;   // ms
    float bodyDecayMs = 500.0f;   // ms
    float noiseAmount = 0.15f;    // 0-1
    float noiseDecayMs = 20.0f;   // ms
    float saturation = 0.3f;      // 0-1
    float click = 0.5f;           // 0-1
    float level = 0.8f;           // 0-1

    // Audio
    double sampleRate = 44100.0;
    juce::Random noiseGen;

    // Auto-trigger state
    bool hasTriggered = false;

    // Helpers
    void updateDecayCoefficients();
    float processSample();
    float softClip(float sample, float drive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KickDesigner)
};
