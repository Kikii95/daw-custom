#include "EnvelopeEffect.h"

EnvelopeEffect::EnvelopeEffect()
{
    updateIncrements();
}

void EnvelopeEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;
    updateIncrements();
}

void EnvelopeEffect::reset()
{
    state = Idle;
    envelopeValue = 0.0f;
    stateProgress = 0.0f;
}

void EnvelopeEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    // Auto-trigger on first process if enabled and idle
    if (autoTrigger && state == Idle)
        trigger();

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Get current envelope value
        float env = processEnvelope();

        // Apply envelope to all channels
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            float sample = block.getSample(static_cast<int>(ch), static_cast<int>(i));
            block.setSample(static_cast<int>(ch), static_cast<int>(i), sample * env);
        }
    }
}

float EnvelopeEffect::processEnvelope()
{
    switch (state)
    {
        case Idle:
            envelopeValue = 0.0f;
            break;

        case Attack:
            envelopeValue += attackIncrement;
            if (envelopeValue >= 1.0f)
            {
                envelopeValue = 1.0f;
                state = Decay;
            }
            break;

        case Decay:
            // Exponential decay towards sustain level
            envelopeValue -= (envelopeValue - sustainLevel) * decayIncrement;
            if (envelopeValue <= sustainLevel + 0.001f)
            {
                envelopeValue = sustainLevel;
                state = Sustain;
            }
            break;

        case Sustain:
            envelopeValue = sustainLevel;
            // Stay here until release() is called
            break;

        case Release:
            // Exponential release towards zero
            envelopeValue -= envelopeValue * releaseIncrement;
            if (envelopeValue <= 0.001f)
            {
                envelopeValue = 0.0f;
                state = Idle;
            }
            break;
    }

    return envelopeValue;
}

void EnvelopeEffect::trigger()
{
    state = Attack;
    // Don't reset envelopeValue to 0 - allows re-triggering mid-envelope
}

void EnvelopeEffect::release()
{
    if (state != Idle)
        state = Release;
}

void EnvelopeEffect::forceIdle()
{
    state = Idle;
    envelopeValue = 0.0f;
}

void EnvelopeEffect::updateIncrements()
{
    // Convert ms to per-sample increments
    // Attack: linear ramp from 0 to 1
    float attackSamples = juce::jmax(1.0f, attackMs * static_cast<float>(sampleRate) / 1000.0f);
    attackIncrement = 1.0f / attackSamples;

    // Decay: exponential factor (reaches target in ~5 time constants)
    float decaySamples = juce::jmax(1.0f, decayMs * static_cast<float>(sampleRate) / 1000.0f);
    decayIncrement = 5.0f / decaySamples;

    // Release: exponential factor
    float releaseSamples = juce::jmax(1.0f, releaseMs * static_cast<float>(sampleRate) / 1000.0f);
    releaseIncrement = 5.0f / releaseSamples;
}

void EnvelopeEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case AttackMs:
            attackMs = juce::jlimit(1.0f, 5000.0f, value);
            updateIncrements();
            break;
        case DecayMs:
            decayMs = juce::jlimit(1.0f, 5000.0f, value);
            updateIncrements();
            break;
        case SustainLevel:
            sustainLevel = juce::jlimit(0.0f, 1.0f, value);
            break;
        case ReleaseMs:
            releaseMs = juce::jlimit(1.0f, 5000.0f, value);
            updateIncrements();
            break;
        case AutoTrigger:
            autoTrigger = value > 0.5f;
            break;
        default:
            break;
    }
}

float EnvelopeEffect::getParameter(int index) const
{
    switch (index)
    {
        case AttackMs:     return attackMs;
        case DecayMs:      return decayMs;
        case SustainLevel: return sustainLevel;
        case ReleaseMs:    return releaseMs;
        case AutoTrigger:  return autoTrigger ? 1.0f : 0.0f;
        default:           return 0.0f;
    }
}

juce::String EnvelopeEffect::getParameterName(int index) const
{
    switch (index)
    {
        case AttackMs:     return "Attack";
        case DecayMs:      return "Decay";
        case SustainLevel: return "Sustain";
        case ReleaseMs:    return "Release";
        case AutoTrigger:  return "Auto";
        default:           return {};
    }
}

float EnvelopeEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case AttackMs:
        case DecayMs:
        case ReleaseMs:    return 1.0f;
        case SustainLevel:
        case AutoTrigger:  return 0.0f;
        default:           return 0.0f;
    }
}

float EnvelopeEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case AttackMs:
        case DecayMs:
        case ReleaseMs:    return 5000.0f;
        case SustainLevel: return 1.0f;
        case AutoTrigger:  return 1.0f;
        default:           return 1.0f;
    }
}

float EnvelopeEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case AttackMs:     return 10.0f;
        case DecayMs:      return 100.0f;
        case SustainLevel: return 0.7f;
        case ReleaseMs:    return 200.0f;
        case AutoTrigger:  return 1.0f;
        default:           return 0.0f;
    }
}
