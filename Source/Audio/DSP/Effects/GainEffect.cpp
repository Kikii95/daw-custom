#include "GainEffect.h"

GainEffect::GainEffect()
{
    smoothedVolume.setCurrentAndTargetValue(1.0f);
    smoothedLeftGain.setCurrentAndTargetValue(1.0f);
    smoothedRightGain.setCurrentAndTargetValue(1.0f);
}

void GainEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;

    // 20ms smoothing for click-free parameter changes
    auto smoothingTime = 0.02;
    smoothedVolume.reset(spec.sampleRate, smoothingTime);
    smoothedLeftGain.reset(spec.sampleRate, smoothingTime);
    smoothedRightGain.reset(spec.sampleRate, smoothingTime);

    updatePanGains();
}

void GainEffect::reset()
{
    smoothedVolume.setCurrentAndTargetValue(volume);
    updatePanGains();
}

void GainEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numChannels = block.getNumChannels();
    auto numSamples = block.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Mono: just apply volume
    if (numChannels == 1)
    {
        auto* data = block.getChannelPointer(0);
        for (size_t i = 0; i < numSamples; ++i)
            data[i] *= smoothedVolume.getNextValue();
    }
    // Stereo: apply volume and pan
    else if (numChannels >= 2)
    {
        auto* left = block.getChannelPointer(0);
        auto* right = block.getChannelPointer(1);

        for (size_t i = 0; i < numSamples; ++i)
        {
            float vol = smoothedVolume.getNextValue();
            float lGain = smoothedLeftGain.getNextValue() * vol;
            float rGain = smoothedRightGain.getNextValue() * vol;

            left[i] *= lGain;
            right[i] *= rGain;
        }

        // Copy to additional channels if present
        for (size_t ch = 2; ch < numChannels; ++ch)
        {
            auto* data = block.getChannelPointer(ch);
            for (size_t i = 0; i < numSamples; ++i)
                data[i] *= volume;
        }
    }
}

void GainEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case Volume:
            setVolume(value);
            break;
        case Pan:
            setPan(value);
            break;
        default:
            break;
    }
}

float GainEffect::getParameter(int index) const
{
    switch (index)
    {
        case Volume: return volume;
        case Pan:    return pan;
        default:     return 0.0f;
    }
}

juce::String GainEffect::getParameterName(int index) const
{
    switch (index)
    {
        case Volume: return "Volume";
        case Pan:    return "Pan";
        default:     return {};
    }
}

float GainEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case Volume: return 0.0f;
        case Pan:    return -1.0f;
        default:     return 0.0f;
    }
}

float GainEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case Volume: return 2.0f;
        case Pan:    return 1.0f;
        default:     return 1.0f;
    }
}

float GainEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case Volume: return 1.0f;
        case Pan:    return 0.0f;
        default:     return 0.0f;
    }
}

void GainEffect::setVolume(float newVolume)
{
    volume = juce::jlimit(0.0f, 2.0f, newVolume);
    smoothedVolume.setTargetValue(volume);
}

void GainEffect::setPan(float newPan)
{
    pan = juce::jlimit(-1.0f, 1.0f, newPan);
    updatePanGains();
}

void GainEffect::updatePanGains()
{
    // Constant power panning
    // pan: -1 = left, 0 = center, +1 = right
    // theta: 0 to pi/2 (left channel angle)
    float theta = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;

    // L = cos(theta), R = sin(theta)
    // At center (pan=0): theta = pi/4, cos = sin = 0.707 (equal power)
    // At left (pan=-1): theta = 0, L = 1, R = 0
    // At right (pan=+1): theta = pi/2, L = 0, R = 1
    smoothedLeftGain.setTargetValue(std::cos(theta));
    smoothedRightGain.setTargetValue(std::sin(theta));
}
