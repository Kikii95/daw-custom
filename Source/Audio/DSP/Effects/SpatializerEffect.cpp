#include "SpatializerEffect.h"

SpatializerEffect::SpatializerEffect()
{
    updateGains();
}

void SpatializerEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;

    // Initialize smoothed values for click-free parameter changes
    smoothedLeftGain.reset(spec.sampleRate, 0.02);   // 20ms ramp
    smoothedRightGain.reset(spec.sampleRate, 0.02);

    updateGains();
    smoothedLeftGain.setCurrentAndTargetValue(leftGain);
    smoothedRightGain.setCurrentAndTargetValue(rightGain);
}

void SpatializerEffect::reset()
{
    smoothedLeftGain.setCurrentAndTargetValue(leftGain);
    smoothedRightGain.setCurrentAndTargetValue(rightGain);
}

void SpatializerEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed || context.isBypassed)
        return;

    auto& block = context.getOutputBlock();
    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    if (numChannels < 2)
        return;  // Need stereo for spatial effect

    // Update target gains
    updateGains();
    smoothedLeftGain.setTargetValue(leftGain * distanceAttenuation);
    smoothedRightGain.setTargetValue(rightGain * distanceAttenuation);

    auto* leftChannel = block.getChannelPointer(0);
    auto* rightChannel = block.getChannelPointer(1);

    for (size_t i = 0; i < numSamples; ++i)
    {
        float lGain = smoothedLeftGain.getNextValue();
        float rGain = smoothedRightGain.getNextValue();

        // Mix both channels based on spatial position
        float mono = (leftChannel[i] + rightChannel[i]) * 0.5f;
        leftChannel[i] = mono * lGain;
        rightChannel[i] = mono * rGain;
    }
}

void SpatializerEffect::updateGains()
{
    float az = azimuth.load();
    float el = elevation.load();
    float dist = distance.load();

    // Constant-power pan based on azimuth
    // az = 0 → center, az = -90 → full left, az = +90 → full right
    float pan = (az + 90.0f) / 180.0f;  // Normalize to 0..1
    pan = juce::jlimit(0.0f, 1.0f, pan);

    // Equal-power panning (constant power)
    leftGain = std::cos(pan * juce::MathConstants<float>::halfPi);
    rightGain = std::sin(pan * juce::MathConstants<float>::halfPi);

    // Elevation factor: slight attenuation for extreme up/down
    float elevFactor = 1.0f - (std::abs(el) / 180.0f) * 0.3f;

    // Distance attenuation using inverse square law
    distanceAttenuation = 1.0f / juce::jmax(0.1f, dist);
    distanceAttenuation = juce::jlimit(0.0f, 1.0f, distanceAttenuation);
    distanceAttenuation *= elevFactor;
}

void SpatializerEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case Azimuth:   setAzimuth(value); break;
        case Elevation: setElevation(value); break;
        case Distance:  setDistance(value); break;
        default: break;
    }
}

float SpatializerEffect::getParameter(int index) const
{
    switch (index)
    {
        case Azimuth:   return azimuth.load();
        case Elevation: return elevation.load();
        case Distance:  return distance.load();
        default:        return 0.0f;
    }
}

juce::String SpatializerEffect::getParameterName(int index) const
{
    switch (index)
    {
        case Azimuth:   return "Azimuth";
        case Elevation: return "Elevation";
        case Distance:  return "Distance";
        default:        return {};
    }
}

float SpatializerEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case Azimuth:   return -180.0f;
        case Elevation: return -90.0f;
        case Distance:  return 0.1f;
        default:        return 0.0f;
    }
}

float SpatializerEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case Azimuth:   return 180.0f;
        case Elevation: return 90.0f;
        case Distance:  return 100.0f;
        default:        return 1.0f;
    }
}

float SpatializerEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case Azimuth:   return 0.0f;    // Center
        case Elevation: return 0.0f;    // Level
        case Distance:  return 1.0f;    // 1 meter
        default:        return 0.0f;
    }
}

void SpatializerEffect::setAzimuth(float degrees)
{
    azimuth.store(juce::jlimit(-180.0f, 180.0f, degrees));
}

void SpatializerEffect::setElevation(float degrees)
{
    elevation.store(juce::jlimit(-90.0f, 90.0f, degrees));
}

void SpatializerEffect::setDistance(float meters)
{
    distance.store(juce::jlimit(0.1f, 100.0f, meters));
}
