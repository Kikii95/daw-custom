#include "EffectChain.h"

void EffectChain::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;

    for (auto& effect : effects)
    {
        if (effect)
            effect->prepare(spec);
    }

    prepared = true;
}

void EffectChain::process(juce::AudioBuffer<float>& buffer)
{
    if (chainBypassed || !prepared)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    for (auto& effect : effects)
    {
        if (effect && !effect->isBypassed())
            effect->process(context);
    }
}

void EffectChain::reset()
{
    for (auto& effect : effects)
    {
        if (effect)
            effect->reset();
    }
}

void EffectChain::addEffect(std::unique_ptr<EffectSlot> effect)
{
    if (!effect)
        return;

    if (prepared)
        effect->prepare(currentSpec);

    effects.push_back(std::move(effect));
}

void EffectChain::insertEffect(int index, std::unique_ptr<EffectSlot> effect)
{
    if (!effect)
        return;

    if (prepared)
        effect->prepare(currentSpec);

    index = juce::jlimit(0, static_cast<int>(effects.size()), index);
    effects.insert(effects.begin() + index, std::move(effect));
}

std::unique_ptr<EffectSlot> EffectChain::removeEffect(int index)
{
    if (index < 0 || index >= static_cast<int>(effects.size()))
        return nullptr;

    auto effect = std::move(effects[static_cast<size_t>(index)]);
    effects.erase(effects.begin() + index);
    return effect;
}

void EffectChain::moveEffect(int fromIndex, int toIndex)
{
    auto numEffects = static_cast<int>(effects.size());

    if (fromIndex < 0 || fromIndex >= numEffects ||
        toIndex < 0 || toIndex >= numEffects ||
        fromIndex == toIndex)
        return;

    auto effect = std::move(effects[static_cast<size_t>(fromIndex)]);
    effects.erase(effects.begin() + fromIndex);

    if (toIndex > fromIndex)
        --toIndex;

    effects.insert(effects.begin() + toIndex, std::move(effect));
}

void EffectChain::clearEffects()
{
    effects.clear();
}

EffectSlot* EffectChain::getEffect(int index)
{
    if (index < 0 || index >= static_cast<int>(effects.size()))
        return nullptr;

    return effects[static_cast<size_t>(index)].get();
}

const EffectSlot* EffectChain::getEffect(int index) const
{
    if (index < 0 || index >= static_cast<int>(effects.size()))
        return nullptr;

    return effects[static_cast<size_t>(index)].get();
}
