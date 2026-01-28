#include "DelayEffect.h"

DelayEffect::DelayEffect()
{
    smoothedDelayL.setCurrentAndTargetValue(delayTimeMs);
    smoothedDelayR.setCurrentAndTargetValue(delayTimeMs);
}

void DelayEffect::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    currentSampleRate = spec.sampleRate;

    delayLineL.prepare(spec);
    delayLineR.prepare(spec);

    // Max delay in samples
    int maxDelaySamples = static_cast<int>(2.0 * spec.sampleRate);
    delayLineL.setMaximumDelayInSamples(maxDelaySamples);
    delayLineR.setMaximumDelayInSamples(maxDelaySamples);

    // 50ms smoothing for delay time changes
    smoothedDelayL.reset(spec.sampleRate, 0.05);
    smoothedDelayR.reset(spec.sampleRate, 0.05);

    updateDelayTime();
}

void DelayEffect::reset()
{
    delayLineL.reset();
    delayLineR.reset();
    smoothedDelayL.setCurrentAndTargetValue(delayTimeMs * static_cast<float>(currentSampleRate) / 1000.0f);
    smoothedDelayR.setCurrentAndTargetValue(delayTimeMs * static_cast<float>(currentSampleRate) / 1000.0f);
}

void DelayEffect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed)
        return;

    auto& block = context.getOutputBlock();
    auto numChannels = block.getNumChannels();
    auto numSamples = block.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Mono processing
    if (numChannels == 1)
    {
        auto* data = block.getChannelPointer(0);

        for (size_t i = 0; i < numSamples; ++i)
        {
            float delaySamples = smoothedDelayL.getNextValue();
            float input = data[i];

            // Read delayed sample
            float delayed = delayLineL.popSample(0, delaySamples);

            // Write to delay line: input + feedback
            // Soft clipping to prevent runaway feedback (from legacy code)
            float toWrite = input + (delayed * feedback);
            toWrite = juce::jlimit(-2.0f, 2.0f, toWrite);
            delayLineL.pushSample(0, toWrite);

            // Output: dry + wet mix
            data[i] = input * (1.0f - dryWetMix) + delayed * dryWetMix;
        }
    }
    // Stereo processing
    else
    {
        auto* left = block.getChannelPointer(0);
        auto* right = block.getChannelPointer(1);

        for (size_t i = 0; i < numSamples; ++i)
        {
            float delaySamplesL = smoothedDelayL.getNextValue();
            float delaySamplesR = smoothedDelayR.getNextValue();

            float inputL = left[i];
            float inputR = right[i];

            // Read delayed samples
            float delayedL = delayLineL.popSample(0, delaySamplesL);
            float delayedR = delayLineR.popSample(0, delaySamplesR);

            // Write to delay lines with feedback + soft clipping
            float toWriteL = inputL + (delayedL * feedback);
            float toWriteR = inputR + (delayedR * feedback);
            toWriteL = juce::jlimit(-2.0f, 2.0f, toWriteL);
            toWriteR = juce::jlimit(-2.0f, 2.0f, toWriteR);

            delayLineL.pushSample(0, toWriteL);
            delayLineR.pushSample(0, toWriteR);

            // Output with dry/wet mix
            left[i] = inputL * (1.0f - dryWetMix) + delayedL * dryWetMix;
            right[i] = inputR * (1.0f - dryWetMix) + delayedR * dryWetMix;
        }
    }
}

void DelayEffect::setParameter(int index, float value)
{
    switch (index)
    {
        case DelayTime:
            setDelayTime(value);
            break;
        case Feedback:
            setFeedback(value);
            break;
        case Mix:
            setDryWetMix(value);
            break;
        default:
            break;
    }
}

float DelayEffect::getParameter(int index) const
{
    switch (index)
    {
        case DelayTime: return delayTimeMs;
        case Feedback:  return feedback;
        case Mix:       return dryWetMix;
        default:        return 0.0f;
    }
}

juce::String DelayEffect::getParameterName(int index) const
{
    switch (index)
    {
        case DelayTime: return "Time";
        case Feedback:  return "Feedback";
        case Mix:       return "Mix";
        default:        return {};
    }
}

float DelayEffect::getParameterMin(int index) const
{
    switch (index)
    {
        case DelayTime: return 0.0f;
        case Feedback:  return 0.0f;
        case Mix:       return 0.0f;
        default:        return 0.0f;
    }
}

float DelayEffect::getParameterMax(int index) const
{
    switch (index)
    {
        case DelayTime: return 2000.0f;
        case Feedback:  return 0.95f;
        case Mix:       return 1.0f;
        default:        return 1.0f;
    }
}

float DelayEffect::getParameterDefault(int index) const
{
    switch (index)
    {
        case DelayTime: return 250.0f;
        case Feedback:  return 0.4f;
        case Mix:       return 0.5f;
        default:        return 0.0f;
    }
}

void DelayEffect::setDelayTime(float timeMs)
{
    delayTimeMs = juce::jlimit(0.0f, 2000.0f, timeMs);
    updateDelayTime();
}

void DelayEffect::setFeedback(float fb)
{
    // Cap at 0.95 to prevent infinite feedback
    feedback = juce::jlimit(0.0f, 0.95f, fb);
}

void DelayEffect::setDryWetMix(float mixValue)
{
    dryWetMix = juce::jlimit(0.0f, 1.0f, mixValue);
}

void DelayEffect::syncToTempo(double bpm, float noteValue)
{
    // noteValue: 1.0 = quarter note, 0.5 = eighth, 2.0 = half, etc.
    if (bpm <= 0.0)
        return;

    double beatDurationMs = 60000.0 / bpm;
    setDelayTime(static_cast<float>(beatDurationMs * noteValue));
}

void DelayEffect::updateDelayTime()
{
    if (currentSampleRate <= 0.0)
        return;

    float delaySamples = delayTimeMs * static_cast<float>(currentSampleRate) / 1000.0f;
    smoothedDelayL.setTargetValue(delaySamples);
    smoothedDelayR.setTargetValue(delaySamples);
}
