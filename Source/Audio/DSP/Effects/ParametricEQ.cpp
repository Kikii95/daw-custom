#include "ParametricEQ.h"

ParametricEQ::ParametricEQ()
{
}

void ParametricEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;

    for (auto& f : lowFilters)
        f.prepare(spec);
    for (auto& f : midFilters)
        f.prepare(spec);
    for (auto& f : highFilters)
        f.prepare(spec);

    updateAllCoefficients();
}

void ParametricEQ::reset()
{
    for (auto& f : lowFilters)
        f.reset();
    for (auto& f : midFilters)
        f.reset();
    for (auto& f : highFilters)
        f.reset();
}

void ParametricEQ::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numChannels = block.getNumChannels();
    auto numSamples = block.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Process each channel through all 3 filter bands
    for (size_t ch = 0; ch < juce::jmin(numChannels, size_t(2)); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> channelContext(channelBlock);

        // Low shelf
        if (std::abs(lowGain) > 0.01f)
            lowFilters[ch].process(channelContext);

        // Mid peak
        if (std::abs(midGain) > 0.01f)
            midFilters[ch].process(channelContext);

        // High shelf
        if (std::abs(highGain) > 0.01f)
            highFilters[ch].process(channelContext);
    }
}

void ParametricEQ::setParameter(int index, float value)
{
    switch (index)
    {
        case LowFreq:
            lowFreq = juce::jlimit(20.0f, 500.0f, value);
            updateLowCoefficients();
            break;
        case LowGain:
            lowGain = juce::jlimit(-24.0f, 24.0f, value);
            updateLowCoefficients();
            break;
        case MidFreq:
            midFreq = juce::jlimit(200.0f, 8000.0f, value);
            updateMidCoefficients();
            break;
        case MidGain:
            midGain = juce::jlimit(-24.0f, 24.0f, value);
            updateMidCoefficients();
            break;
        case MidQ:
            midQ = juce::jlimit(0.1f, 10.0f, value);
            updateMidCoefficients();
            break;
        case HighFreq:
            highFreq = juce::jlimit(2000.0f, 20000.0f, value);
            updateHighCoefficients();
            break;
        case HighGain:
            highGain = juce::jlimit(-24.0f, 24.0f, value);
            updateHighCoefficients();
            break;
        default:
            break;
    }
}

float ParametricEQ::getParameter(int index) const
{
    switch (index)
    {
        case LowFreq:  return lowFreq;
        case LowGain:  return lowGain;
        case MidFreq:  return midFreq;
        case MidGain:  return midGain;
        case MidQ:     return midQ;
        case HighFreq: return highFreq;
        case HighGain: return highGain;
        default:       return 0.0f;
    }
}

juce::String ParametricEQ::getParameterName(int index) const
{
    switch (index)
    {
        case LowFreq:  return "Low Freq";
        case LowGain:  return "Low Gain";
        case MidFreq:  return "Mid Freq";
        case MidGain:  return "Mid Gain";
        case MidQ:     return "Mid Q";
        case HighFreq: return "High Freq";
        case HighGain: return "High Gain";
        default:       return {};
    }
}

float ParametricEQ::getParameterMin(int index) const
{
    switch (index)
    {
        case LowFreq:  return 20.0f;
        case MidFreq:  return 200.0f;
        case HighFreq: return 2000.0f;
        case LowGain:
        case MidGain:
        case HighGain: return -24.0f;
        case MidQ:     return 0.1f;
        default:       return 0.0f;
    }
}

float ParametricEQ::getParameterMax(int index) const
{
    switch (index)
    {
        case LowFreq:  return 500.0f;
        case MidFreq:  return 8000.0f;
        case HighFreq: return 20000.0f;
        case LowGain:
        case MidGain:
        case HighGain: return 24.0f;
        case MidQ:     return 10.0f;
        default:       return 1.0f;
    }
}

float ParametricEQ::getParameterDefault(int index) const
{
    switch (index)
    {
        case LowFreq:  return 200.0f;
        case MidFreq:  return 1000.0f;
        case HighFreq: return 5000.0f;
        case LowGain:
        case MidGain:
        case HighGain: return 0.0f;
        case MidQ:     return 1.0f;
        default:       return 0.0f;
    }
}

void ParametricEQ::setLowShelf(float freq, float gainDb)
{
    lowFreq = juce::jlimit(20.0f, 500.0f, freq);
    lowGain = juce::jlimit(-24.0f, 24.0f, gainDb);
    updateLowCoefficients();
}

void ParametricEQ::setMidPeak(float freq, float gainDb, float q)
{
    midFreq = juce::jlimit(200.0f, 8000.0f, freq);
    midGain = juce::jlimit(-24.0f, 24.0f, gainDb);
    midQ = juce::jlimit(0.1f, 10.0f, q);
    updateMidCoefficients();
}

void ParametricEQ::setHighShelf(float freq, float gainDb)
{
    highFreq = juce::jlimit(2000.0f, 20000.0f, freq);
    highGain = juce::jlimit(-24.0f, 24.0f, gainDb);
    updateHighCoefficients();
}

void ParametricEQ::updateLowCoefficients()
{
    if (currentSpec.sampleRate <= 0)
        return;

    auto coeffs = Coefficients::makeLowShelf(
        currentSpec.sampleRate,
        lowFreq,
        1.0f,
        juce::Decibels::decibelsToGain(lowGain)
    );

    for (auto& f : lowFilters)
        *f.coefficients = *coeffs;
}

void ParametricEQ::updateMidCoefficients()
{
    if (currentSpec.sampleRate <= 0)
        return;

    auto coeffs = Coefficients::makePeakFilter(
        currentSpec.sampleRate,
        midFreq,
        midQ,
        juce::Decibels::decibelsToGain(midGain)
    );

    for (auto& f : midFilters)
        *f.coefficients = *coeffs;
}

void ParametricEQ::updateHighCoefficients()
{
    if (currentSpec.sampleRate <= 0)
        return;

    auto coeffs = Coefficients::makeHighShelf(
        currentSpec.sampleRate,
        highFreq,
        1.0f,
        juce::Decibels::decibelsToGain(highGain)
    );

    for (auto& f : highFilters)
        *f.coefficients = *coeffs;
}

void ParametricEQ::updateAllCoefficients()
{
    updateLowCoefficients();
    updateMidCoefficients();
    updateHighCoefficients();
}
