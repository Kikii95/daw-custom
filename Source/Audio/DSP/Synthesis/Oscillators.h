#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

/**
 * Waveform generation helpers for synthesis.
 * All functions expect normalized phase in range [0, 1).
 */
namespace Synthesis
{
    enum class Waveform
    {
        Sine = 0,
        Square,
        Sawtooth,
        Triangle,
        Noise
    };

    /**
     * Generate sine wave sample.
     * @param phase Normalized phase [0, 1)
     * @return Sample value [-1, 1]
     */
    inline float sine(float phase)
    {
        return std::sin(phase * juce::MathConstants<float>::twoPi);
    }

    /**
     * Generate square wave sample (naive, with aliasing).
     * @param phase Normalized phase [0, 1)
     * @return Sample value {-1, 1}
     */
    inline float square(float phase)
    {
        return phase < 0.5f ? 1.0f : -1.0f;
    }

    /**
     * Generate sawtooth wave sample (naive, with aliasing).
     * @param phase Normalized phase [0, 1)
     * @return Sample value [-1, 1]
     */
    inline float sawtooth(float phase)
    {
        return 2.0f * phase - 1.0f;
    }

    /**
     * Generate triangle wave sample.
     * @param phase Normalized phase [0, 1)
     * @return Sample value [-1, 1]
     */
    inline float triangle(float phase)
    {
        return 4.0f * std::abs(phase - 0.5f) - 1.0f;
    }

    /**
     * Generate white noise sample.
     * @param rng Random number generator
     * @return Sample value [-1, 1]
     */
    inline float noise(juce::Random& rng)
    {
        return rng.nextFloat() * 2.0f - 1.0f;
    }

    /**
     * Generate sample for given waveform type.
     * @param waveform Waveform type
     * @param phase Normalized phase [0, 1)
     * @param rng Random generator (for noise)
     * @return Sample value [-1, 1]
     */
    inline float generate(Waveform waveform, float phase, juce::Random& rng)
    {
        switch (waveform)
        {
            case Waveform::Sine:     return sine(phase);
            case Waveform::Square:   return square(phase);
            case Waveform::Sawtooth: return sawtooth(phase);
            case Waveform::Triangle: return triangle(phase);
            case Waveform::Noise:    return noise(rng);
            default:                 return 0.0f;
        }
    }

    /**
     * Get waveform name for display.
     */
    inline juce::String getWaveformName(Waveform waveform)
    {
        switch (waveform)
        {
            case Waveform::Sine:     return "Sine";
            case Waveform::Square:   return "Square";
            case Waveform::Sawtooth: return "Saw";
            case Waveform::Triangle: return "Triangle";
            case Waveform::Noise:    return "Noise";
            default:                 return "Unknown";
        }
    }

    /**
     * Number of available waveforms.
     */
    constexpr int numWaveforms = 5;
}
