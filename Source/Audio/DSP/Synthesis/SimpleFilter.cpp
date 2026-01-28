#include "SimpleFilter.h"
#include <cmath>

SimpleFilter::SimpleFilter()
{
    smoothedCutoff.setCurrentAndTargetValue(cutoffHz);
}

void SimpleFilter::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;

    // Prepare filters
    filterL.prepare(spec);
    filterR.prepare(spec);

    // Setup smoothing (10ms ramp)
    smoothedCutoff.reset(sampleRate, 0.01);
    smoothedCutoff.setCurrentAndTargetValue(cutoffHz);

    updateFilter();
}

void SimpleFilter::reset()
{
    filterL.reset();
    filterR.reset();
    envelopeValue = 0.0f;
}

void SimpleFilter::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numSamples = block.getNumSamples();
    auto numChannels = block.getNumChannels();

    // Process sample by sample for smooth modulation
    for (size_t i = 0; i < numSamples; ++i)
    {
        // Update cutoff with smoothing and envelope modulation
        float targetCutoff = calculateModulatedCutoff();
        smoothedCutoff.setTargetValue(targetCutoff);
        float currentCutoff = smoothedCutoff.getNextValue();

        // Update filter cutoff
        filterL.setCutoffFrequency(currentCutoff);
        filterR.setCutoffFrequency(currentCutoff);

        // Process each channel
        if (numChannels >= 1)
        {
            float sampleL = block.getSample(0, static_cast<int>(i));
            float outL = 0.0f;

            switch (mode)
            {
                case LowPass:
                    outL = filterL.processSample(0, sampleL);
                    break;
                case HighPass:
                    outL = filterL.processSample(1, sampleL);
                    break;
                case BandPass:
                    outL = filterL.processSample(2, sampleL);
                    break;
            }

            block.setSample(0, static_cast<int>(i), outL);
        }

        if (numChannels >= 2)
        {
            float sampleR = block.getSample(1, static_cast<int>(i));
            float outR = 0.0f;

            switch (mode)
            {
                case LowPass:
                    outR = filterR.processSample(0, sampleR);
                    break;
                case HighPass:
                    outR = filterR.processSample(1, sampleR);
                    break;
                case BandPass:
                    outR = filterR.processSample(2, sampleR);
                    break;
            }

            block.setSample(1, static_cast<int>(i), outR);
        }
    }
}

float SimpleFilter::calculateModulatedCutoff()
{
    // Envelope modulation in octaves
    // envAmount = 1 means +4 octaves at env=1
    // envAmount = -1 means -4 octaves at env=1
    float octaveShift = envAmount * envelopeValue * 4.0f;
    float modulatedCutoff = cutoffHz * std::pow(2.0f, octaveShift);

    return juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
}

void SimpleFilter::updateFilter()
{
    // Map resonance 0-1 to Q 0.5-10
    float q = 0.5f + resonance * 9.5f;

    filterL.setResonance(q);
    filterR.setResonance(q);

    filterL.setCutoffFrequency(cutoffHz);
    filterR.setCutoffFrequency(cutoffHz);
}

void SimpleFilter::setParameter(int index, float value)
{
    switch (index)
    {
        case Cutoff:
            cutoffHz = juce::jlimit(20.0f, 20000.0f, value);
            updateFilter();
            break;
        case Resonance:
            resonance = juce::jlimit(0.0f, 1.0f, value);
            updateFilter();
            break;
        case Mode:
            mode = static_cast<FilterMode>(juce::jlimit(0, 2, static_cast<int>(value)));
            break;
        case EnvAmount:
            envAmount = juce::jlimit(-1.0f, 1.0f, value);
            break;
        default:
            break;
    }
}

float SimpleFilter::getParameter(int index) const
{
    switch (index)
    {
        case Cutoff:    return cutoffHz;
        case Resonance: return resonance;
        case Mode:      return static_cast<float>(mode);
        case EnvAmount: return envAmount;
        default:        return 0.0f;
    }
}

juce::String SimpleFilter::getParameterName(int index) const
{
    switch (index)
    {
        case Cutoff:    return "Cutoff";
        case Resonance: return "Reso";
        case Mode:      return "Mode";
        case EnvAmount: return "EnvAmt";
        default:        return {};
    }
}

float SimpleFilter::getParameterMin(int index) const
{
    switch (index)
    {
        case Cutoff:    return 20.0f;
        case Resonance: return 0.0f;
        case Mode:      return 0.0f;
        case EnvAmount: return -1.0f;
        default:        return 0.0f;
    }
}

float SimpleFilter::getParameterMax(int index) const
{
    switch (index)
    {
        case Cutoff:    return 20000.0f;
        case Resonance: return 1.0f;
        case Mode:      return 2.0f;
        case EnvAmount: return 1.0f;
        default:        return 1.0f;
    }
}

float SimpleFilter::getParameterDefault(int index) const
{
    switch (index)
    {
        case Cutoff:    return 1000.0f;
        case Resonance: return 0.0f;
        case Mode:      return 0.0f;  // LowPass
        case EnvAmount: return 0.0f;
        default:        return 0.0f;
    }
}
