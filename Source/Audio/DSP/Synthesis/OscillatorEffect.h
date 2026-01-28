#pragma once

#include "../EffectSlot.h"
#include "Oscillators.h"
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * Oscillator effect that generates audio.
 * Can be added to a track's effect chain to produce sound.
 * Supports multiple waveforms and continuous frequency control.
 */
class OscillatorEffect : public EffectSlot
{
public:
    enum Parameter
    {
        Frequency = 0,   // Hz (20-20000)
        WaveformType,    // 0-4 (Sine, Square, Saw, Triangle, Noise)
        Level,           // 0-1
        NumParams
    };

    OscillatorEffect();
    ~OscillatorEffect() override = default;

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
    juce::String getName() const override { return "Oscillator"; }
    juce::String getDescription() const override { return "Audio oscillator (sine, square, saw, tri, noise)"; }

    // Direct control
    void setFrequency(float hz);
    void setWaveform(Synthesis::Waveform wf);
    void setLevel(float lvl);

    float getFrequency() const { return frequency; }
    Synthesis::Waveform getWaveform() const { return waveform; }
    float getLevel() const { return level; }

private:
    // Oscillator state
    float phase = 0.0f;
    float frequency = 440.0f;
    Synthesis::Waveform waveform = Synthesis::Waveform::Sine;
    float level = 0.5f;

    // Smoothing for click-free parameter changes
    juce::SmoothedValue<float> smoothedFreq;
    juce::SmoothedValue<float> smoothedLevel;

    // For noise generation
    juce::Random noiseGen;

    // Sample rate
    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorEffect)
};
