#pragma once

#include "Preset.h"
#include "../Audio/DSP/EffectSlot.h"
#include <map>

/**
 * Manages effect presets: scanning, saving, loading, and applying.
 *
 * Preset directory structure:
 *   ~/Music/DAWCustom/Presets/
 *   ├── Effects/
 *   │   ├── EQ/
 *   │   ├── Compressor/
 *   │   └── ...
 *   └── Synths/
 *       ├── KickDesigner/
 *       └── BasicSynth/
 */
class PresetManager
{
public:
    PresetManager();
    ~PresetManager() = default;

    // Scanning
    void scanPresets();
    void rescanPresets() { scanPresets(); }

    // Get presets for specific effect type
    juce::Array<Preset> getPresetsForEffect(const juce::String& effectType) const;

    // Get all categories
    juce::StringArray getCategories() const;

    // Get all effect types that have presets
    juce::StringArray getEffectTypes() const;

    // Save/Load
    bool savePreset(const Preset& preset);
    bool savePreset(const Preset& preset, const juce::File& targetFile);

    // Create preset from current effect state
    Preset createFromEffect(const EffectSlot* effect, const juce::String& name,
                           const juce::String& category = "User") const;

    // Apply preset to effect
    bool applyPreset(const Preset& preset, EffectSlot* effect) const;

    // Directory access
    juce::File getPresetsDirectory() const;
    juce::File getEffectPresetsDirectory(const juce::String& effectType) const;

    // Factory presets
    void createFactoryPresets();
    bool hasFactoryPresets() const;

private:
    // Presets organized by effect type
    std::map<juce::String, juce::Array<Preset>> presetsByType;

    // Helper: scan a directory for presets
    void scanDirectory(const juce::File& dir, const juce::String& effectType);

    // Helper: ensure directories exist
    void ensureDirectoriesExist();

    // Helper: normalize effect type name for filesystem
    static juce::String normalizeEffectType(const juce::String& type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
