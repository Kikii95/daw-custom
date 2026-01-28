#include "KickDesigner.h"
#include <cmath>

KickDesigner::KickDesigner()
{
    updateDecayCoefficients();
}

void KickDesigner::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;
    updateDecayCoefficients();
}

void KickDesigner::reset()
{
    phase = 0.0f;
    pitchEnvValue = 0.0f;
    ampEnvValue = 0.0f;
    noiseEnvValue = 0.0f;
    clickEnvValue = 0.0f;
    hasTriggered = false;
}

void KickDesigner::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    // Auto-trigger on first process if not triggered yet
    if (!hasTriggered)
    {
        trigger();
        hasTriggered = true;
    }

    for (size_t i = 0; i < numSamples; ++i)
    {
        float sample = processSample();

        // Output to all channels (mono kick)
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            block.setSample(static_cast<int>(ch), static_cast<int>(i), sample);
        }
    }
}

float KickDesigner::processSample()
{
    // Early exit if kick has fully decayed
    if (ampEnvValue < 0.0001f && pitchEnvValue < 0.0001f)
        return 0.0f;

    // === Pitch Envelope ===
    // Exponential decay from (baseFreq * pitchMultiplier) to baseFreq
    pitchEnvValue *= pitchDecayCoeff;

    // Calculate pitch multiplier from semitones
    // pitchAmount semitones up when env=1, baseFreq when env=0
    float pitchMultiplier = std::pow(2.0f, (pitchAmount * pitchEnvValue) / 12.0f);
    currentFreq = baseFreq * pitchMultiplier;

    // === Main Sine Oscillator ===
    float sine = Synthesis::sine(phase);

    // Advance phase
    phase += static_cast<float>(currentFreq / sampleRate);
    if (phase >= 1.0f)
        phase -= 1.0f;

    // === Click (initial transient) ===
    clickEnvValue *= clickDecayCoeff;
    float clickSample = clickEnvValue * click * 2.0f;  // Boost click

    // === Noise Layer ===
    noiseEnvValue *= noiseDecayCoeff;
    float noise = Synthesis::noise(noiseGen) * noiseEnvValue * noiseAmount;

    // === Mix ===
    float sample = sine + noise + clickSample;

    // === Saturation (soft clip) ===
    if (saturation > 0.0f)
        sample = softClip(sample, saturation);

    // === Amplitude Envelope ===
    ampEnvValue *= ampDecayCoeff;
    sample *= ampEnvValue;

    // === Output Level ===
    sample *= level;

    return sample;
}

float KickDesigner::softClip(float sample, float drive)
{
    // Increase gain before clipping based on drive amount
    float gain = 1.0f + drive * 4.0f;  // 1x to 5x gain
    sample *= gain;

    // Soft clip using tanh
    sample = std::tanh(sample);

    // Compensate for gain increase
    sample /= std::tanh(gain);

    return sample;
}

void KickDesigner::trigger()
{
    // Reset phase for consistent attack
    phase = 0.0f;

    // Start all envelopes at 1.0
    pitchEnvValue = 1.0f;
    ampEnvValue = 1.0f;
    noiseEnvValue = 1.0f;
    clickEnvValue = 1.0f;
}

void KickDesigner::updateDecayCoefficients()
{
    // Convert ms to per-sample decay coefficient
    // coefficient = e^(-1 / (time_in_samples))
    // For ~5 time constants to reach near-zero:
    // coefficient = e^(-5 / samples)

    auto msToCoeff = [this](float ms, float timeConstants = 5.0f) -> float
    {
        float samples = juce::jmax(1.0f, ms * static_cast<float>(sampleRate) / 1000.0f);
        return std::exp(-timeConstants / samples);
    };

    pitchDecayCoeff = msToCoeff(pitchDecayMs);
    ampDecayCoeff = msToCoeff(bodyDecayMs);
    noiseDecayCoeff = msToCoeff(noiseDecayMs);

    // Click is always very fast (2-5ms)
    clickDecayCoeff = msToCoeff(3.0f);
}

void KickDesigner::setParameter(int index, float value)
{
    switch (index)
    {
        case BaseFreq:
            baseFreq = juce::jlimit(40.0f, 120.0f, value);
            break;
        case PitchAmount:
            pitchAmount = juce::jlimit(0.0f, 48.0f, value);
            break;
        case PitchDecay:
            pitchDecayMs = juce::jlimit(5.0f, 500.0f, value);
            updateDecayCoefficients();
            break;
        case BodyDecay:
            bodyDecayMs = juce::jlimit(50.0f, 2000.0f, value);
            updateDecayCoefficients();
            break;
        case NoiseAmount:
            noiseAmount = juce::jlimit(0.0f, 1.0f, value);
            break;
        case NoiseDecay:
            noiseDecayMs = juce::jlimit(1.0f, 100.0f, value);
            updateDecayCoefficients();
            break;
        case Saturation:
            saturation = juce::jlimit(0.0f, 1.0f, value);
            break;
        case Click:
            click = juce::jlimit(0.0f, 1.0f, value);
            break;
        case Level:
            level = juce::jlimit(0.0f, 1.0f, value);
            break;
        default:
            break;
    }
}

float KickDesigner::getParameter(int index) const
{
    switch (index)
    {
        case BaseFreq:     return baseFreq;
        case PitchAmount:  return pitchAmount;
        case PitchDecay:   return pitchDecayMs;
        case BodyDecay:    return bodyDecayMs;
        case NoiseAmount:  return noiseAmount;
        case NoiseDecay:   return noiseDecayMs;
        case Saturation:   return saturation;
        case Click:        return click;
        case Level:        return level;
        default:           return 0.0f;
    }
}

juce::String KickDesigner::getParameterName(int index) const
{
    switch (index)
    {
        case BaseFreq:     return "Freq";
        case PitchAmount:  return "Pitch";
        case PitchDecay:   return "P.Decay";
        case BodyDecay:    return "Decay";
        case NoiseAmount:  return "Noise";
        case NoiseDecay:   return "N.Decay";
        case Saturation:   return "Drive";
        case Click:        return "Click";
        case Level:        return "Level";
        default:           return {};
    }
}

float KickDesigner::getParameterMin(int index) const
{
    switch (index)
    {
        case BaseFreq:     return 40.0f;
        case PitchAmount:  return 0.0f;
        case PitchDecay:   return 5.0f;
        case BodyDecay:    return 50.0f;
        case NoiseAmount:  return 0.0f;
        case NoiseDecay:   return 1.0f;
        case Saturation:   return 0.0f;
        case Click:        return 0.0f;
        case Level:        return 0.0f;
        default:           return 0.0f;
    }
}

float KickDesigner::getParameterMax(int index) const
{
    switch (index)
    {
        case BaseFreq:     return 120.0f;
        case PitchAmount:  return 48.0f;
        case PitchDecay:   return 500.0f;
        case BodyDecay:    return 2000.0f;
        case NoiseAmount:  return 1.0f;
        case NoiseDecay:   return 100.0f;
        case Saturation:   return 1.0f;
        case Click:        return 1.0f;
        case Level:        return 1.0f;
        default:           return 1.0f;
    }
}

float KickDesigner::getParameterDefault(int index) const
{
    switch (index)
    {
        case BaseFreq:     return 60.0f;    // Classic 808 fundamental
        case PitchAmount:  return 36.0f;    // 3 octaves pitch drop
        case PitchDecay:   return 50.0f;    // Fast pitch decay
        case BodyDecay:    return 500.0f;   // Medium body
        case NoiseAmount:  return 0.15f;    // Subtle noise
        case NoiseDecay:   return 20.0f;    // Short noise burst
        case Saturation:   return 0.3f;     // Light saturation
        case Click:        return 0.5f;     // Medium click
        case Level:        return 0.8f;     // Headroom
        default:           return 0.0f;
    }
}
