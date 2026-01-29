#include "PresetManager.h"
#include "../Audio/Plugins/VST3EffectSlot.h"

PresetManager::PresetManager()
{
    ensureDirectoriesExist();
    scanPresets();
}

void PresetManager::ensureDirectoriesExist()
{
    auto presetsDir = getPresetsDirectory();
    presetsDir.createDirectory();

    // Create subdirectories
    presetsDir.getChildFile("Effects").createDirectory();
    presetsDir.getChildFile("Synths").createDirectory();

    // Create effect type directories
    juce::StringArray effectTypes = {
        "Gain", "EQ", "Compressor", "Reverb", "Delay",
        "Oscillator", "Envelope"
    };

    for (const auto& type : effectTypes)
    {
        auto typeDir = presetsDir.getChildFile("Effects").getChildFile(type);
        typeDir.createDirectory();
    }

    // Synth directories
    juce::StringArray synthTypes = { "KickDesigner", "BasicSynth" };
    for (const auto& type : synthTypes)
    {
        auto typeDir = presetsDir.getChildFile("Synths").getChildFile(type);
        typeDir.createDirectory();
    }
}

juce::File PresetManager::getPresetsDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userMusicDirectory)
        .getChildFile("DAWCustom")
        .getChildFile("Presets");
}

juce::File PresetManager::getEffectPresetsDirectory(const juce::String& effectType) const
{
    auto normalized = normalizeEffectType(effectType);

    // Check if it's a synth
    if (normalized == "KickDesigner" || normalized == "BasicSynth")
        return getPresetsDirectory().getChildFile("Synths").getChildFile(normalized);

    return getPresetsDirectory().getChildFile("Effects").getChildFile(normalized);
}

juce::String PresetManager::normalizeEffectType(const juce::String& type)
{
    // Map effect names to directory names
    if (type == "Parametric EQ" || type == "EQ")
        return "EQ";
    if (type == "Kick Designer")
        return "KickDesigner";
    if (type == "Basic Synth")
        return "BasicSynth";

    return type;
}

void PresetManager::scanPresets()
{
    presetsByType.clear();

    auto presetsDir = getPresetsDirectory();

    // Scan Effects directory
    auto effectsDir = presetsDir.getChildFile("Effects");
    for (const auto& typeDir : effectsDir.findChildFiles(juce::File::findDirectories, false))
    {
        scanDirectory(typeDir, typeDir.getFileName());
    }

    // Scan Synths directory
    auto synthsDir = presetsDir.getChildFile("Synths");
    for (const auto& typeDir : synthsDir.findChildFiles(juce::File::findDirectories, false))
    {
        scanDirectory(typeDir, typeDir.getFileName());
    }

    // Scan VST3 directory
    auto vst3Dir = presetsDir.getChildFile("VST3");
    if (vst3Dir.isDirectory())
    {
        for (const auto& pluginDir : vst3Dir.findChildFiles(juce::File::findDirectories, false))
        {
            scanDirectory(pluginDir, "VST3:" + pluginDir.getFileName());
        }
    }
}

void PresetManager::scanDirectory(const juce::File& dir, const juce::String& effectType)
{
    auto jsonFiles = dir.findChildFiles(juce::File::findFiles, false, "*.json");

    for (const auto& file : jsonFiles)
    {
        auto preset = Preset::loadFromFile(file);
        if (preset.isValid())
        {
            presetsByType[effectType].add(preset);
        }
    }
}

juce::Array<Preset> PresetManager::getPresetsForEffect(const juce::String& effectType) const
{
    auto normalized = normalizeEffectType(effectType);

    auto it = presetsByType.find(normalized);
    if (it != presetsByType.end())
        return it->second;

    return {};
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray categories;

    for (const auto& [type, presets] : presetsByType)
    {
        for (const auto& preset : presets)
        {
            if (preset.category.isNotEmpty() && !categories.contains(preset.category))
                categories.add(preset.category);
        }
    }

    categories.sort(false);
    return categories;
}

juce::StringArray PresetManager::getEffectTypes() const
{
    juce::StringArray types;

    for (const auto& [type, presets] : presetsByType)
    {
        if (!presets.isEmpty())
            types.add(type);
    }

    types.sort(false);
    return types;
}

bool PresetManager::savePreset(const Preset& preset)
{
    auto dir = getEffectPresetsDirectory(preset.effectType);
    auto file = dir.getChildFile(preset.name + ".json");
    return savePreset(preset, file);
}

bool PresetManager::savePreset(const Preset& preset, const juce::File& targetFile)
{
    if (preset.saveToFile(targetFile))
    {
        // Re-scan to update internal list
        scanPresets();
        return true;
    }
    return false;
}

Preset PresetManager::createFromEffect(const EffectSlot* effect, const juce::String& name,
                                       const juce::String& category) const
{
    Preset preset;
    preset.name = name;
    preset.category = category;
    preset.effectType = effect->getName();
    preset.author = "User";

    // Check if VST3
    if (auto* vst = dynamic_cast<const VST3EffectSlot*>(effect))
    {
        preset.effectType = "VST3";
        preset.pluginId = vst->getPluginDescription().createIdentifierString();

        juce::MemoryBlock state;
        const_cast<VST3EffectSlot*>(vst)->getStateInformation(state);
        preset.vst3State = state.toBase64Encoding();
    }
    else
    {
        // Native effect - store all parameters
        for (int i = 0; i < effect->getNumParameters(); ++i)
        {
            auto paramName = effect->getParameterName(i);
            preset.parameters[paramName] = effect->getParameter(i);
        }
    }

    return preset;
}

bool PresetManager::applyPreset(const Preset& preset, EffectSlot* effect) const
{
    if (effect == nullptr || !preset.isValid())
        return false;

    // VST3 handling
    if (auto* vst = dynamic_cast<VST3EffectSlot*>(effect))
    {
        if (preset.vst3State.isEmpty())
            return false;

        juce::MemoryBlock state;
        state.fromBase64Encoding(preset.vst3State);
        vst->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        return true;
    }

    // Native effect - apply parameters by name
    for (const auto& [paramName, value] : preset.parameters)
    {
        for (int i = 0; i < effect->getNumParameters(); ++i)
        {
            if (effect->getParameterName(i) == paramName)
            {
                effect->setParameter(i, value);
                break;
            }
        }
    }

    return true;
}

bool PresetManager::hasFactoryPresets() const
{
    for (const auto& [type, presets] : presetsByType)
    {
        if (!presets.isEmpty())
            return true;
    }
    return false;
}

void PresetManager::createFactoryPresets()
{
    // EQ Presets
    {
        Preset preset;
        preset.name = "Vocal Presence";
        preset.category = "Vocal";
        preset.effectType = "EQ";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"lowGain", -2.0f}, {"lowFreq", 80.0f},
            {"midGain", 3.0f}, {"midFreq", 2500.0f}, {"midQ", 1.2f},
            {"highGain", 2.0f}, {"highFreq", 10000.0f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Bass Cut";
        preset.category = "Utility";
        preset.effectType = "EQ";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"lowGain", -12.0f}, {"lowFreq", 100.0f},
            {"midGain", 0.0f}, {"midFreq", 1000.0f}, {"midQ", 1.0f},
            {"highGain", 0.0f}, {"highFreq", 8000.0f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Bright Mix";
        preset.category = "Master";
        preset.effectType = "EQ";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"lowGain", 0.0f}, {"lowFreq", 60.0f},
            {"midGain", -1.0f}, {"midFreq", 500.0f}, {"midQ", 0.8f},
            {"highGain", 4.0f}, {"highFreq", 12000.0f}
        };
        savePreset(preset);
    }

    // Compressor Presets
    {
        Preset preset;
        preset.name = "Gentle Glue";
        preset.category = "Bus";
        preset.effectType = "Compressor";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"threshold", -18.0f}, {"ratio", 2.0f},
            {"attack", 30.0f}, {"release", 200.0f}, {"makeup", 2.0f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Punchy Drums";
        preset.category = "Drums";
        preset.effectType = "Compressor";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"threshold", -12.0f}, {"ratio", 4.0f},
            {"attack", 5.0f}, {"release", 80.0f}, {"makeup", 4.0f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Vocal Leveler";
        preset.category = "Vocal";
        preset.effectType = "Compressor";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"threshold", -20.0f}, {"ratio", 3.0f},
            {"attack", 10.0f}, {"release", 150.0f}, {"makeup", 3.0f}
        };
        savePreset(preset);
    }

    // Reverb Presets
    {
        Preset preset;
        preset.name = "Small Room";
        preset.category = "Room";
        preset.effectType = "Reverb";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"roomSize", 0.3f}, {"damping", 0.5f},
            {"wet", 0.25f}, {"dry", 0.75f}, {"width", 0.8f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Large Hall";
        preset.category = "Hall";
        preset.effectType = "Reverb";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"roomSize", 0.85f}, {"damping", 0.3f},
            {"wet", 0.35f}, {"dry", 0.65f}, {"width", 1.0f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Plate Vocal";
        preset.category = "Vocal";
        preset.effectType = "Reverb";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"roomSize", 0.6f}, {"damping", 0.4f},
            {"wet", 0.2f}, {"dry", 0.8f}, {"width", 0.9f}
        };
        savePreset(preset);
    }

    // Delay Presets
    {
        Preset preset;
        preset.name = "Slapback";
        preset.category = "Vocal";
        preset.effectType = "Delay";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"delayTime", 0.08f}, {"feedback", 0.2f}, {"mix", 0.3f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Quarter Note";
        preset.category = "Rhythmic";
        preset.effectType = "Delay";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"delayTime", 0.5f}, {"feedback", 0.4f}, {"mix", 0.25f}
        };
        savePreset(preset);
    }

    // KickDesigner Presets
    {
        Preset preset;
        preset.name = "808 Classic";
        preset.category = "Hip-Hop";
        preset.effectType = "KickDesigner";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"pitch", 55.0f}, {"pitchDecay", 0.15f}, {"click", 0.3f},
            {"saturation", 0.4f}, {"attack", 0.001f}, {"decay", 0.3f},
            {"sustain", 0.2f}, {"release", 0.5f}, {"output", 0.8f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Punchy House";
        preset.category = "House";
        preset.effectType = "KickDesigner";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"pitch", 60.0f}, {"pitchDecay", 0.08f}, {"click", 0.6f},
            {"saturation", 0.2f}, {"attack", 0.001f}, {"decay", 0.15f},
            {"sustain", 0.1f}, {"release", 0.2f}, {"output", 0.85f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Sub Bass";
        preset.category = "Bass";
        preset.effectType = "KickDesigner";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"pitch", 40.0f}, {"pitchDecay", 0.25f}, {"click", 0.1f},
            {"saturation", 0.6f}, {"attack", 0.005f}, {"decay", 0.6f},
            {"sustain", 0.4f}, {"release", 0.8f}, {"output", 0.7f}
        };
        savePreset(preset);
    }

    // BasicSynth Presets
    {
        Preset preset;
        preset.name = "Warm Pad";
        preset.category = "Pad";
        preset.effectType = "BasicSynth";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"osc1Wave", 0.0f}, {"osc1Tune", 0.0f},
            {"osc2Wave", 0.0f}, {"osc2Tune", -12.0f}, {"oscMix", 0.5f},
            {"filterType", 0.0f}, {"cutoff", 800.0f}, {"resonance", 0.3f},
            {"attack", 0.5f}, {"decay", 0.3f}, {"sustain", 0.7f}, {"release", 1.0f}
        };
        savePreset(preset);
    }

    {
        Preset preset;
        preset.name = "Bright Lead";
        preset.category = "Lead";
        preset.effectType = "BasicSynth";
        preset.author = "DAW Custom";
        preset.parameters = {
            {"osc1Wave", 1.0f}, {"osc1Tune", 0.0f},
            {"osc2Wave", 2.0f}, {"osc2Tune", 7.0f}, {"oscMix", 0.6f},
            {"filterType", 0.0f}, {"cutoff", 2000.0f}, {"resonance", 0.4f},
            {"attack", 0.01f}, {"decay", 0.1f}, {"sustain", 0.8f}, {"release", 0.3f}
        };
        savePreset(preset);
    }

    DBG("Factory presets created");
}
