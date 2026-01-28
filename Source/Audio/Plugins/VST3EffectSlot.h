#pragma once

#include "../DSP/EffectSlot.h"
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Wrapper adapting a VST3 AudioPluginInstance to the EffectSlot interface.
 *
 * This allows VST3 plugins to be used in the existing EffectChain infrastructure
 * alongside native effects (EQ, Compressor, etc.).
 *
 * Signal flow:
 *   Input buffer → AudioPluginInstance::processBlock() → Output buffer
 */
class VST3EffectSlot : public EffectSlot
{
public:
    explicit VST3EffectSlot(std::unique_ptr<juce::AudioPluginInstance> pluginInstance);
    ~VST3EffectSlot() override;

    // EffectSlot interface
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    // Parameter interface (maps to plugin parameters)
    void setParameter(int index, float value) override;
    float getParameter(int index) const override;
    juce::String getParameterName(int index) const override;
    int getNumParameters() const override;

    // Parameter ranges
    float getParameterMin(int index) const override;
    float getParameterMax(int index) const override;
    float getParameterDefault(int index) const override;

    // Effect info
    juce::String getName() const override;
    juce::String getDescription() const override;

    // Plugin-specific methods
    juce::AudioPluginInstance* getPluginInstance() { return plugin.get(); }
    const juce::AudioPluginInstance* getPluginInstance() const { return plugin.get(); }

    // Editor support
    bool hasEditor() const;
    juce::AudioProcessorEditor* createEditor();

    // Plugin state (for preset save/load)
    void getStateInformation(juce::MemoryBlock& destData);
    void setStateInformation(const void* data, int sizeInBytes);

    // Latency (for host compensation)
    int getLatencySamples() const;

    // Plugin description
    const juce::PluginDescription& getPluginDescription() const { return description; }

private:
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::PluginDescription description;

    // MIDI buffer (required by processBlock even for effects)
    juce::MidiBuffer emptyMidiBuffer;

    // Cached parameter list for faster access
    juce::Array<juce::AudioProcessorParameter*> cachedParams;
    void cacheParameters();

    // Audio spec
    double sampleRate = 44100.0;
    int blockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST3EffectSlot)
};
