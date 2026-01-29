#include "TremoloEffect.h"

TremoloEffect::TremoloEffect()
{
}

void TremoloEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;
    updatePhaseIncrement();
}

void TremoloEffect::reset()
{
    phase = 0.0f;
}

void TremoloEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numChannels = block.getNumChannels();
    auto numSamples = block.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    auto easingFunc = Tweening::GetFunction(shape);

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Get LFO value from easing function (0-1)
        float lfoValue = easingFunc(phase);

        // Calculate gain: 1.0 at depth=0, modulated at depth=1
        float gain = 1.0f - (lfoValue * depth);

        // Apply to all channels
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* data = block.getChannelPointer(ch);
            data[i] *= gain;
        }

        // Advance LFO phase
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}

void TremoloEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case Rate:
            setRate(value);
            break;
        case Depth:
            setDepth(value);
            break;
        case Shape:
            setShapeIndex(static_cast<int>(value));
            break;
        default:
            break;
    }
}

float TremoloEffect::getParameter(int index) const
{
    switch (index)
    {
        case Rate:  return rate;
        case Depth: return depth;
        case Shape: return static_cast<float>(shape);
        default:    return 0.0f;
    }
}

juce::String TremoloEffect::getParameterName(int index) const
{
    switch (index)
    {
        case Rate:  return "Rate";
        case Depth: return "Depth";
        case Shape: return "Shape";
        default:    return {};
    }
}

float TremoloEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case Rate:  return 0.1f;
        case Depth: return 0.0f;
        case Shape: return 0.0f;
        default:    return 0.0f;
    }
}

float TremoloEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case Rate:  return 20.0f;
        case Depth: return 1.0f;
        case Shape: return 30.0f;
        default:    return 1.0f;
    }
}

float TremoloEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case Rate:  return 4.0f;
        case Depth: return 0.5f;
        case Shape: return static_cast<float>(Tweening::EasingType::EaseInOutSine);
        default:    return 0.0f;
    }
}

void TremoloEffect::setRate(float hz)
{
    rate = juce::jlimit(0.1f, 20.0f, hz);
    updatePhaseIncrement();
}

void TremoloEffect::setDepth(float d)
{
    depth = juce::jlimit(0.0f, 1.0f, d);
}

void TremoloEffect::setShape(Tweening::EasingType type)
{
    shape = type;
}

void TremoloEffect::setShapeIndex(int index)
{
    if (index >= 0 && index <= 30)
        shape = static_cast<Tweening::EasingType>(index);
}

void TremoloEffect::updatePhaseIncrement()
{
    if (sampleRate > 0.0)
        phaseIncrement = static_cast<float>(rate / sampleRate);
}

juce::String TremoloEffect::getShapeName(int index)
{
    static const char* names[] = {
        "Linear",
        "Sine In", "Sine Out", "Sine InOut",
        "Cubic In", "Cubic Out", "Cubic InOut",
        "Quint In", "Quint Out", "Quint InOut",
        "Circ In", "Circ Out", "Circ InOut",
        "Elastic In", "Elastic Out", "Elastic InOut",
        "Quad In", "Quad Out", "Quad InOut",
        "Quart In", "Quart Out", "Quart InOut",
        "Expo In", "Expo Out", "Expo InOut",
        "Back In", "Back Out", "Back InOut",
        "Bounce In", "Bounce Out", "Bounce InOut"
    };

    if (index >= 0 && index < 31)
        return names[index];
    return "Unknown";
}
