#include "VST3EffectSlot.h"

VST3EffectSlot::VST3EffectSlot(std::unique_ptr<juce::AudioPluginInstance> pluginInstance)
    : plugin(std::move(pluginInstance))
{
    jassert(plugin != nullptr);

    // Store description for later use
    plugin->fillInPluginDescription(description);

    // Cache parameters for faster access
    cacheParameters();

    DBG("VST3EffectSlot: Loaded " + getName());
}

VST3EffectSlot::~VST3EffectSlot()
{
    // Plugin will be released automatically
}

void VST3EffectSlot::cacheParameters()
{
    cachedParams.clear();

    if (plugin)
    {
        auto& params = plugin->getParameters();
        for (auto* param : params)
        {
            // Only include automatable parameters
            if (param->isAutomatable())
                cachedParams.add(param);
        }
    }
}

void VST3EffectSlot::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = spec.sampleRate;
    blockSize = static_cast<int>(spec.maximumBlockSize);

    if (plugin)
    {
        plugin->setPlayConfigDetails(
            static_cast<int>(spec.numChannels),
            static_cast<int>(spec.numChannels),
            spec.sampleRate,
            static_cast<int>(spec.maximumBlockSize));

        plugin->prepareToPlay(spec.sampleRate, static_cast<int>(spec.maximumBlockSize));
    }
}

void VST3EffectSlot::reset()
{
    if (plugin)
        plugin->reset();
}

void VST3EffectSlot::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (bypassed || !plugin)
        return;

    auto& block = context.getOutputBlock();
    auto numChannels = static_cast<int>(block.getNumChannels());
    auto numSamples = static_cast<int>(block.getNumSamples());

    // Build array of channel pointers from AudioBlock
    float* channelPtrs[32];  // Max 32 channels
    jassert(numChannels <= 32);

    for (int ch = 0; ch < numChannels; ++ch)
        channelPtrs[ch] = block.getChannelPointer(static_cast<size_t>(ch));

    // Create audio buffer wrapping the block data
    juce::AudioBuffer<float> buffer(channelPtrs, numChannels, numSamples);

    // Process through plugin
    plugin->processBlock(buffer, emptyMidiBuffer);
}

void VST3EffectSlot::setParameter(int index, float value)
{
    if (index >= 0 && index < cachedParams.size())
    {
        auto* param = cachedParams[index];
        // Value is expected in 0-1 range
        param->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, value));
    }
}

float VST3EffectSlot::getParameter(int index) const
{
    if (index >= 0 && index < cachedParams.size())
    {
        return cachedParams[index]->getValue();
    }
    return 0.0f;
}

juce::String VST3EffectSlot::getParameterName(int index) const
{
    if (index >= 0 && index < cachedParams.size())
    {
        return cachedParams[index]->getName(32);  // Max 32 chars
    }
    return {};
}

int VST3EffectSlot::getNumParameters() const
{
    return cachedParams.size();
}

float VST3EffectSlot::getParameterMin(int /*index*/) const
{
    // VST parameters are normalized 0-1
    return 0.0f;
}

float VST3EffectSlot::getParameterMax(int /*index*/) const
{
    // VST parameters are normalized 0-1
    return 1.0f;
}

float VST3EffectSlot::getParameterDefault(int index) const
{
    if (index >= 0 && index < cachedParams.size())
    {
        return cachedParams[index]->getDefaultValue();
    }
    return 0.5f;
}

juce::String VST3EffectSlot::getName() const
{
    if (plugin)
        return plugin->getName();
    return "Unknown Plugin";
}

juce::String VST3EffectSlot::getDescription() const
{
    if (plugin)
    {
        return description.manufacturerName + " - " + description.descriptiveName;
    }
    return "VST3 Plugin";
}

bool VST3EffectSlot::hasEditor() const
{
    return plugin && plugin->hasEditor();
}

juce::AudioProcessorEditor* VST3EffectSlot::createEditor()
{
    if (plugin && plugin->hasEditor())
        return plugin->createEditor();
    return nullptr;
}

void VST3EffectSlot::getStateInformation(juce::MemoryBlock& destData)
{
    if (plugin)
        plugin->getStateInformation(destData);
}

void VST3EffectSlot::setStateInformation(const void* data, int sizeInBytes)
{
    if (plugin)
        plugin->setStateInformation(data, sizeInBytes);
}

int VST3EffectSlot::getLatencySamples() const
{
    if (plugin)
        return plugin->getLatencySamples();
    return 0;
}
