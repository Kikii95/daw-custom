#include "BasicSynth.h"
#include <cmath>

BasicSynth::BasicSynth()
{
    updateEnvelopeIncrements();
}

void BasicSynth::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;

    // Prepare filter
    filterL.prepare(spec);
    filterR.prepare(spec);
    filterL.setResonance(0.5f + filterReso * 9.5f);
    filterR.setResonance(0.5f + filterReso * 9.5f);

    // Setup smoothing
    smoothedFreq.reset(sampleRate, 0.005);  // 5ms glide
    smoothedFreq.setCurrentAndTargetValue(frequency);

    smoothedCutoff.reset(sampleRate, 0.01);
    smoothedCutoff.setCurrentAndTargetValue(filterCutoff);

    updateEnvelopeIncrements();
}

void BasicSynth::reset()
{
    phase1 = 0.0f;
    phase2 = 0.0f;
    state = EnvIdle;
    ampEnvValue = 0.0f;
    filterEnvValue = 0.0f;
    filterL.reset();
    filterR.reset();
    hasTriggered = false;
}

void BasicSynth::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    // Auto-trigger on first process
    if (!hasTriggered)
    {
        trigger();
        hasTriggered = true;
    }

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Process envelope
        float env = processEnvelope();

        // Skip if envelope is silent
        if (env < 0.0001f && state == EnvIdle)
        {
            for (size_t ch = 0; ch < numChannels; ++ch)
                block.setSample(static_cast<int>(ch), static_cast<int>(i), 0.0f);
            continue;
        }

        // Get smoothed frequency
        smoothedFreq.setTargetValue(frequency);
        float currentFreq = smoothedFreq.getNextValue();

        // Generate oscillator 1
        float osc1 = generateOscillator(phase1, currentFreq);

        // Generate oscillator 2 with detune
        float detuneRatio = std::pow(2.0f, osc2Detune / 1200.0f);  // Cents to ratio
        float freq2 = currentFreq * detuneRatio;
        float osc2 = generateOscillator(phase2, freq2);

        // Mix oscillators
        float oscMix = osc1 * (1.0f - osc2Mix) + osc2 * osc2Mix;

        // Update filter with envelope modulation
        updateFilter(filterEnvValue);
        float currentCutoff = smoothedCutoff.getNextValue();
        filterL.setCutoffFrequency(currentCutoff);
        filterR.setCutoffFrequency(currentCutoff);

        // Apply filter (lowpass)
        float filtered = filterL.processSample(0, oscMix);

        // Apply amplitude envelope and level
        float sample = filtered * env * level;

        // Output to all channels
        for (size_t ch = 0; ch < numChannels; ++ch)
            block.setSample(static_cast<int>(ch), static_cast<int>(i), sample);
    }
}

float BasicSynth::generateOscillator(float& phase, float freq)
{
    float sample = 0.0f;

    switch (waveform)
    {
        case Synthesis::Waveform::Sine:
            sample = Synthesis::sine(phase);
            break;
        case Synthesis::Waveform::Square:
            sample = Synthesis::square(phase);
            break;
        case Synthesis::Waveform::Sawtooth:
            sample = Synthesis::sawtooth(phase);
            break;
        case Synthesis::Waveform::Triangle:
            sample = Synthesis::triangle(phase);
            break;
        case Synthesis::Waveform::Noise:
            sample = Synthesis::noise(noiseGen);
            break;
    }

    // Advance phase
    phase += static_cast<float>(freq / sampleRate);
    if (phase >= 1.0f)
        phase -= 1.0f;

    return sample;
}

float BasicSynth::processEnvelope()
{
    switch (state)
    {
        case EnvIdle:
            ampEnvValue = 0.0f;
            filterEnvValue = 0.0f;
            break;

        case EnvAttack:
            ampEnvValue += attackInc;
            filterEnvValue = ampEnvValue;
            if (ampEnvValue >= 1.0f)
            {
                ampEnvValue = 1.0f;
                filterEnvValue = 1.0f;
                state = EnvDecay;
            }
            break;

        case EnvDecay:
            // Exponential decay towards sustain
            ampEnvValue -= (ampEnvValue - sustainLevel) * decayInc;
            filterEnvValue = ampEnvValue;
            if (ampEnvValue <= sustainLevel + 0.001f)
            {
                ampEnvValue = sustainLevel;
                state = EnvSustain;
            }
            break;

        case EnvSustain:
            ampEnvValue = sustainLevel;
            filterEnvValue = sustainLevel;
            break;

        case EnvRelease:
            ampEnvValue -= ampEnvValue * releaseInc;
            filterEnvValue = ampEnvValue;
            if (ampEnvValue <= 0.001f)
            {
                ampEnvValue = 0.0f;
                filterEnvValue = 0.0f;
                state = EnvIdle;
            }
            break;
    }

    return ampEnvValue;
}

void BasicSynth::updateFilter(float envValue)
{
    // Modulate cutoff with envelope
    // envAmount controls how many octaves the envelope opens the filter
    float envOctaves = filterEnvAmt * envValue * 4.0f;  // Up to 4 octaves
    float modulatedCutoff = filterCutoff * std::pow(2.0f, envOctaves);
    modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);

    smoothedCutoff.setTargetValue(modulatedCutoff);
}

void BasicSynth::trigger()
{
    state = EnvAttack;
    // Don't reset phases for smoother re-triggering
}

void BasicSynth::release()
{
    if (state != EnvIdle)
        state = EnvRelease;
}

void BasicSynth::updateEnvelopeIncrements()
{
    // Linear attack
    float attackSamples = juce::jmax(1.0f, attackMs * static_cast<float>(sampleRate) / 1000.0f);
    attackInc = 1.0f / attackSamples;

    // Exponential decay/release
    float decaySamples = juce::jmax(1.0f, decayMs * static_cast<float>(sampleRate) / 1000.0f);
    decayInc = 5.0f / decaySamples;

    float releaseSamples = juce::jmax(1.0f, releaseMs * static_cast<float>(sampleRate) / 1000.0f);
    releaseInc = 5.0f / releaseSamples;
}

void BasicSynth::setParameter(int index, float value)
{
    switch (index)
    {
        case Frequency:
            frequency = juce::jlimit(20.0f, 2000.0f, value);
            break;
        case Waveform:
            waveform = static_cast<Synthesis::Waveform>(juce::jlimit(0, 4, static_cast<int>(value)));
            break;
        case Osc2Detune:
            osc2Detune = juce::jlimit(-24.0f, 24.0f, value);
            break;
        case Osc2Mix:
            osc2Mix = juce::jlimit(0.0f, 1.0f, value);
            break;
        case FilterCutoff:
            filterCutoff = juce::jlimit(20.0f, 20000.0f, value);
            break;
        case FilterReso:
            filterReso = juce::jlimit(0.0f, 1.0f, value);
            filterL.setResonance(0.5f + filterReso * 9.5f);
            filterR.setResonance(0.5f + filterReso * 9.5f);
            break;
        case FilterEnvAmt:
            filterEnvAmt = juce::jlimit(0.0f, 1.0f, value);
            break;
        case Attack:
            attackMs = juce::jlimit(1.0f, 5000.0f, value);
            updateEnvelopeIncrements();
            break;
        case Decay:
            decayMs = juce::jlimit(1.0f, 5000.0f, value);
            updateEnvelopeIncrements();
            break;
        case Sustain:
            sustainLevel = juce::jlimit(0.0f, 1.0f, value);
            break;
        case Release:
            releaseMs = juce::jlimit(1.0f, 5000.0f, value);
            updateEnvelopeIncrements();
            break;
        case Level:
            level = juce::jlimit(0.0f, 1.0f, value);
            break;
        default:
            break;
    }
}

float BasicSynth::getParameter(int index) const
{
    switch (index)
    {
        case Frequency:     return frequency;
        case Waveform:      return static_cast<float>(waveform);
        case Osc2Detune:    return osc2Detune;
        case Osc2Mix:       return osc2Mix;
        case FilterCutoff:  return filterCutoff;
        case FilterReso:    return filterReso;
        case FilterEnvAmt:  return filterEnvAmt;
        case Attack:        return attackMs;
        case Decay:         return decayMs;
        case Sustain:       return sustainLevel;
        case Release:       return releaseMs;
        case Level:         return level;
        default:            return 0.0f;
    }
}

juce::String BasicSynth::getParameterName(int index) const
{
    switch (index)
    {
        case Frequency:     return "Freq";
        case Waveform:      return "Wave";
        case Osc2Detune:    return "Detune";
        case Osc2Mix:       return "Osc2";
        case FilterCutoff:  return "Cutoff";
        case FilterReso:    return "Reso";
        case FilterEnvAmt:  return "FltEnv";
        case Attack:        return "Atk";
        case Decay:         return "Dec";
        case Sustain:       return "Sus";
        case Release:       return "Rel";
        case Level:         return "Level";
        default:            return {};
    }
}

float BasicSynth::getParameterMin(int index) const
{
    switch (index)
    {
        case Frequency:     return 20.0f;
        case Waveform:      return 0.0f;
        case Osc2Detune:    return -24.0f;
        case Osc2Mix:       return 0.0f;
        case FilterCutoff:  return 20.0f;
        case FilterReso:    return 0.0f;
        case FilterEnvAmt:  return 0.0f;
        case Attack:        return 1.0f;
        case Decay:         return 1.0f;
        case Sustain:       return 0.0f;
        case Release:       return 1.0f;
        case Level:         return 0.0f;
        default:            return 0.0f;
    }
}

float BasicSynth::getParameterMax(int index) const
{
    switch (index)
    {
        case Frequency:     return 2000.0f;
        case Waveform:      return 4.0f;
        case Osc2Detune:    return 24.0f;
        case Osc2Mix:       return 1.0f;
        case FilterCutoff:  return 20000.0f;
        case FilterReso:    return 1.0f;
        case FilterEnvAmt:  return 1.0f;
        case Attack:        return 5000.0f;
        case Decay:         return 5000.0f;
        case Sustain:       return 1.0f;
        case Release:       return 5000.0f;
        case Level:         return 1.0f;
        default:            return 1.0f;
    }
}

float BasicSynth::getParameterDefault(int index) const
{
    switch (index)
    {
        case Frequency:     return 220.0f;   // A3
        case Waveform:      return 2.0f;     // Sawtooth
        case Osc2Detune:    return 5.0f;     // Slight detune for thickness
        case Osc2Mix:       return 0.5f;     // Equal mix
        case FilterCutoff:  return 2000.0f;  // Warm but not muddy
        case FilterReso:    return 0.3f;     // Subtle resonance
        case FilterEnvAmt:  return 0.5f;     // Medium envelope mod
        case Attack:        return 10.0f;    // Snappy attack
        case Decay:         return 200.0f;   // Medium decay
        case Sustain:       return 0.7f;     // Good sustain
        case Release:       return 300.0f;   // Natural release
        case Level:         return 0.7f;     // Headroom
        default:            return 0.0f;
    }
}
