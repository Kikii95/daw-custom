#include "OscillatorEffect.h"

OscillatorEffect::OscillatorEffect()
{
    smoothedFreq.setCurrentAndTargetValue(frequency);
    smoothedLevel.setCurrentAndTargetValue(level);
}

void OscillatorEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;

    // Set smoothing time (20ms for click-free changes)
    smoothedFreq.reset(sampleRate, 0.02);
    smoothedLevel.reset(sampleRate, 0.02);

    smoothedFreq.setCurrentAndTargetValue(frequency);
    smoothedLevel.setCurrentAndTargetValue(level);
}

void OscillatorEffect::reset()
{
    phase = 0.0f;
    smoothedFreq.setCurrentAndTargetValue(frequency);
    smoothedLevel.setCurrentAndTargetValue(level);
}

void OscillatorEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Get smoothed values
        float currentFreq = smoothedFreq.getNextValue();
        float currentLevel = smoothedLevel.getNextValue();

        // Generate sample
        float sample = Synthesis::generate(waveform, phase, noiseGen) * currentLevel;

        // Output to all channels (mono oscillator, same output to L/R)
        for (size_t ch = 0; ch < numChannels; ++ch)
            block.setSample(static_cast<int>(ch), static_cast<int>(i), sample);

        // Advance phase
        float phaseInc = currentFreq / static_cast<float>(sampleRate);
        phase += phaseInc;

        // Wrap phase to [0, 1)
        while (phase >= 1.0f)
            phase -= 1.0f;
    }
}

void OscillatorEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case Frequency:
            setFrequency(value);
            break;
        case WaveformType:
            setWaveform(static_cast<Synthesis::Waveform>(
                juce::jlimit(0, Synthesis::numWaveforms - 1, static_cast<int>(value))));
            break;
        case Level:
            setLevel(value);
            break;
        default:
            break;
    }
}

float OscillatorEffect::getParameter(int index) const
{
    switch (index)
    {
        case Frequency:    return frequency;
        case WaveformType: return static_cast<float>(waveform);
        case Level:        return level;
        default:           return 0.0f;
    }
}

juce::String OscillatorEffect::getParameterName(int index) const
{
    switch (index)
    {
        case Frequency:    return "Frequency";
        case WaveformType: return "Waveform";
        case Level:        return "Level";
        default:           return {};
    }
}

float OscillatorEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case Frequency:    return 20.0f;
        case WaveformType: return 0.0f;
        case Level:        return 0.0f;
        default:           return 0.0f;
    }
}

float OscillatorEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case Frequency:    return 2000.0f;  // Limit to 2kHz for practical use
        case WaveformType: return static_cast<float>(Synthesis::numWaveforms - 1);
        case Level:        return 1.0f;
        default:           return 1.0f;
    }
}

float OscillatorEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case Frequency:    return 440.0f;
        case WaveformType: return 0.0f;  // Sine
        case Level:        return 0.5f;
        default:           return 0.0f;
    }
}

void OscillatorEffect::setFrequency(float hz)
{
    frequency = juce::jlimit(20.0f, 20000.0f, hz);
    smoothedFreq.setTargetValue(frequency);
}

void OscillatorEffect::setWaveform(Synthesis::Waveform wf)
{
    waveform = wf;
}

void OscillatorEffect::setLevel(float lvl)
{
    level = juce::jlimit(0.0f, 1.0f, lvl);
    smoothedLevel.setTargetValue(level);
}
