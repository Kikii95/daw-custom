#include "ReverbEffect.h"

ReverbEffect::ReverbEffect()
{
    // Default: medium room
    params.roomSize = 0.5f;
    params.damping = 0.5f;
    params.width = 1.0f;
    params.wetLevel = 0.33f;
    params.dryLevel = 0.7f;
    params.freezeMode = 0.0f;
}

void ReverbEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    reverb.prepare(spec);
    updateReverbParams();
}

void ReverbEffect::reset()
{
    reverb.reset();
}

void ReverbEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    reverb.process(context);
}

void ReverbEffect::setParameter(int index, float value)
{
    value = juce::jlimit(0.0f, 1.0f, value);

    switch (index)
    {
        case RoomSize:
            params.roomSize = value;
            break;
        case Damping:
            params.damping = value;
            break;
        case Width:
            params.width = value;
            break;
        case WetLevel:
            params.wetLevel = value;
            break;
        case DryLevel:
            params.dryLevel = value;
            break;
        default:
            return;
    }

    updateReverbParams();
}

float ReverbEffect::getParameter(int index) const
{
    switch (index)
    {
        case RoomSize: return params.roomSize;
        case Damping:  return params.damping;
        case Width:    return params.width;
        case WetLevel: return params.wetLevel;
        case DryLevel: return params.dryLevel;
        default:       return 0.0f;
    }
}

juce::String ReverbEffect::getParameterName(int index) const
{
    switch (index)
    {
        case RoomSize: return "Room Size";
        case Damping:  return "Damping";
        case Width:    return "Width";
        case WetLevel: return "Wet";
        case DryLevel: return "Dry";
        default:       return {};
    }
}

float ReverbEffect::getParameterMin(int /*index*/) const
{
    return 0.0f;
}

float ReverbEffect::getParameterMax(int /*index*/) const
{
    return 1.0f;
}

float ReverbEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case RoomSize: return 0.5f;
        case Damping:  return 0.5f;
        case Width:    return 1.0f;
        case WetLevel: return 0.33f;
        case DryLevel: return 0.7f;
        default:       return 0.0f;
    }
}

void ReverbEffect::setRoomSize(float size)
{
    params.roomSize = juce::jlimit(0.0f, 1.0f, size);
    updateReverbParams();
}

void ReverbEffect::setDamping(float damping)
{
    params.damping = juce::jlimit(0.0f, 1.0f, damping);
    updateReverbParams();
}

void ReverbEffect::setWidth(float width)
{
    params.width = juce::jlimit(0.0f, 1.0f, width);
    updateReverbParams();
}

void ReverbEffect::setWetLevel(float wet)
{
    params.wetLevel = juce::jlimit(0.0f, 1.0f, wet);
    updateReverbParams();
}

void ReverbEffect::setDryLevel(float dry)
{
    params.dryLevel = juce::jlimit(0.0f, 1.0f, dry);
    updateReverbParams();
}

void ReverbEffect::setSmallRoom()
{
    params.roomSize = 0.3f;
    params.damping = 0.7f;
    params.width = 0.5f;
    params.wetLevel = 0.25f;
    params.dryLevel = 0.75f;
    updateReverbParams();
}

void ReverbEffect::setLargeHall()
{
    params.roomSize = 0.9f;
    params.damping = 0.3f;
    params.width = 1.0f;
    params.wetLevel = 0.4f;
    params.dryLevel = 0.6f;
    updateReverbParams();
}

void ReverbEffect::setPlate()
{
    params.roomSize = 0.7f;
    params.damping = 0.4f;
    params.width = 0.8f;
    params.wetLevel = 0.35f;
    params.dryLevel = 0.65f;
    updateReverbParams();
}

void ReverbEffect::updateReverbParams()
{
    reverb.setParameters(params);
}
