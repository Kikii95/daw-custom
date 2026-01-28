#include "CompressorEffect.h"

CompressorEffect::CompressorEffect()
{
    updateCompressorParams();
}

void CompressorEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    compressor.prepare(spec);
    makeup.prepare(spec);
    updateCompressorParams();
}

void CompressorEffect::reset()
{
    compressor.reset();
    makeup.reset();
    gainReduction = 0.0f;
}

void CompressorEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numChannels = block.getNumChannels();
    auto numSamples = block.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Measure input peak for gain reduction calculation
    float inputPeak = 0.0f;
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer(ch);
        for (size_t i = 0; i < numSamples; ++i)
            inputPeak = std::max(inputPeak, std::abs(data[i]));
    }

    // Process through compressor
    compressor.process(context);

    // Measure output peak for gain reduction calculation
    float outputPeak = 0.0f;
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer(ch);
        for (size_t i = 0; i < numSamples; ++i)
            outputPeak = std::max(outputPeak, std::abs(data[i]));
    }

    // Apply makeup gain
    makeup.process(context);

    // Calculate gain reduction (for metering)
    if (inputPeak > 0.0001f)
    {
        float gr = juce::Decibels::gainToDecibels(outputPeak / inputPeak);
        gainReduction = std::min(0.0f, gr);
    }
    else
    {
        gainReduction = 0.0f;
    }
}

void CompressorEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case Threshold:
            setThreshold(value);
            break;
        case Ratio:
            setRatio(value);
            break;
        case Attack:
            setAttack(value);
            break;
        case Release:
            setRelease(value);
            break;
        case MakeupGain:
            setMakeupGain(value);
            break;
        default:
            break;
    }
}

float CompressorEffect::getParameter(int index) const
{
    switch (index)
    {
        case Threshold:  return threshold;
        case Ratio:      return ratio;
        case Attack:     return attack;
        case Release:    return release;
        case MakeupGain: return makeupGain;
        default:         return 0.0f;
    }
}

juce::String CompressorEffect::getParameterName(int index) const
{
    switch (index)
    {
        case Threshold:  return "Threshold";
        case Ratio:      return "Ratio";
        case Attack:     return "Attack";
        case Release:    return "Release";
        case MakeupGain: return "Makeup";
        default:         return {};
    }
}

float CompressorEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case Threshold:  return -60.0f;
        case Ratio:      return 1.0f;
        case Attack:     return 0.1f;
        case Release:    return 10.0f;
        case MakeupGain: return 0.0f;
        default:         return 0.0f;
    }
}

float CompressorEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case Threshold:  return 0.0f;
        case Ratio:      return 20.0f;
        case Attack:     return 100.0f;
        case Release:    return 1000.0f;
        case MakeupGain: return 24.0f;
        default:         return 1.0f;
    }
}

float CompressorEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case Threshold:  return -20.0f;
        case Ratio:      return 4.0f;
        case Attack:     return 10.0f;
        case Release:    return 100.0f;
        case MakeupGain: return 0.0f;
        default:         return 0.0f;
    }
}

void CompressorEffect::setThreshold(float thresholdDb)
{
    threshold = juce::jlimit(-60.0f, 0.0f, thresholdDb);
    compressor.setThreshold(threshold);
}

void CompressorEffect::setRatio(float newRatio)
{
    ratio = juce::jlimit(1.0f, 20.0f, newRatio);
    compressor.setRatio(ratio);
}

void CompressorEffect::setAttack(float attackMs)
{
    attack = juce::jlimit(0.1f, 100.0f, attackMs);
    compressor.setAttack(attack);
}

void CompressorEffect::setRelease(float releaseMs)
{
    release = juce::jlimit(10.0f, 1000.0f, releaseMs);
    compressor.setRelease(release);
}

void CompressorEffect::setMakeupGain(float gainDb)
{
    makeupGain = juce::jlimit(0.0f, 24.0f, gainDb);
    makeup.setGainDecibels(makeupGain);
}

void CompressorEffect::updateCompressorParams()
{
    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setAttack(attack);
    compressor.setRelease(release);
    makeup.setGainDecibels(makeupGain);
}
