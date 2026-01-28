#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "EffectSlot.h"
#include <vector>
#include <memory>

/**
 * Container for multiple EffectSlots processed in series.
 * Manages prepare/process/reset lifecycle for all contained effects.
 */
class EffectChain
{
public:
    EffectChain() = default;
    ~EffectChain() = default;

    // Lifecycle
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Effect management
    void addEffect(std::unique_ptr<EffectSlot> effect);
    void insertEffect(int index, std::unique_ptr<EffectSlot> effect);
    std::unique_ptr<EffectSlot> removeEffect(int index);
    void moveEffect(int fromIndex, int toIndex);
    void clearEffects();

    // Access
    EffectSlot* getEffect(int index);
    const EffectSlot* getEffect(int index) const;
    int getNumEffects() const { return static_cast<int>(effects.size()); }

    // Bypass entire chain
    void setBypass(bool shouldBypass) { chainBypassed = shouldBypass; }
    bool isBypassed() const { return chainBypassed; }

    // Check if prepared
    bool isPrepared() const { return prepared; }

private:
    std::vector<std::unique_ptr<EffectSlot>> effects;
    juce::dsp::ProcessSpec currentSpec { 0, 0, 0 };
    bool prepared = false;
    bool chainBypassed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectChain)
};
