#pragma once

#include <juce_core/juce_core.h>
#include <map>

/**
 * Data structure representing a saved effect preset.
 *
 * Presets can store:
 * - Native effect parameters (as name/value pairs)
 * - VST3 state (as base64 encoded binary)
 */
struct Preset
{
    // Metadata
    juce::String name;
    juce::String author;
    juce::String category;
    juce::String effectType;

    // Version for future compatibility
    juce::String version { "1.0" };

    // File location (empty for factory presets)
    juce::File file;

    // Parameters for native effects
    std::map<juce::String, float> parameters;

    // State for VST3 plugins (base64 encoded)
    juce::String vst3State;

    // Plugin identifier for VST3
    juce::String pluginId;

    // Helpers
    bool isValid() const { return name.isNotEmpty() && effectType.isNotEmpty(); }
    bool isVST3() const { return effectType == "VST3" || pluginId.isNotEmpty(); }
    bool isFactory() const { return !file.existsAsFile(); }

    // Serialize to JSON
    juce::var toJson() const
    {
        auto obj = new juce::DynamicObject();

        obj->setProperty("version", version);
        obj->setProperty("name", name);
        obj->setProperty("author", author);
        obj->setProperty("category", category);
        obj->setProperty("effectType", effectType);

        if (isVST3())
        {
            obj->setProperty("pluginId", pluginId);
            obj->setProperty("state", vst3State);
        }
        else
        {
            auto params = new juce::DynamicObject();
            for (const auto& [key, value] : parameters)
                params->setProperty(key, static_cast<double>(value));
            obj->setProperty("parameters", juce::var(params));
        }

        return juce::var(obj);
    }

    // Deserialize from JSON
    static Preset fromJson(const juce::var& json)
    {
        Preset preset;

        preset.version = json.getProperty("version", "1.0").toString();
        preset.name = json.getProperty("name", "").toString();
        preset.author = json.getProperty("author", "").toString();
        preset.category = json.getProperty("category", "").toString();
        preset.effectType = json.getProperty("effectType", "").toString();

        // VST3 specific
        preset.pluginId = json.getProperty("pluginId", "").toString();
        preset.vst3State = json.getProperty("state", "").toString();

        // Native effect parameters
        auto paramsJson = json.getProperty("parameters", juce::var());
        if (paramsJson.isObject())
        {
            auto* paramsObj = paramsJson.getDynamicObject();
            if (paramsObj != nullptr)
            {
                for (const auto& prop : paramsObj->getProperties())
                    preset.parameters[prop.name.toString()] = static_cast<float>(prop.value);
            }
        }

        return preset;
    }

    // Save to file
    bool saveToFile(const juce::File& targetFile) const
    {
        auto json = toJson();
        auto jsonString = juce::JSON::toString(json, true);
        return targetFile.replaceWithText(jsonString);
    }

    // Load from file
    static Preset loadFromFile(const juce::File& sourceFile)
    {
        Preset preset;

        if (!sourceFile.existsAsFile())
            return preset;

        auto jsonString = sourceFile.loadFileAsString();
        auto json = juce::JSON::parse(jsonString);

        if (json.isObject())
        {
            preset = fromJson(json);
            preset.file = sourceFile;
        }

        return preset;
    }
};
