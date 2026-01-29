#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "EffectSlotComponent.h"
#include "Audio/AudioTrack.h"
#include "Audio/Plugins/PluginManager.h"
#include "Presets/PresetManager.h"
#include "UI/Presets/PresetBrowser.h"
#include <vector>
#include <memory>
#include <functional>

/**
 * Panel containing effect slots for a single track.
 * Displays add effect combo, and list of effect slot components.
 */
class EffectRackPanel : public juce::Component
{
public:
    EffectRackPanel();
    ~EffectRackPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Track binding
    void setTrack(AudioTrack* track);
    AudioTrack* getTrack() const { return currentTrack; }
    void refreshEffects();

    // Tempo for delay sync
    void setTempo(double bpm);

    // Callbacks to MainComponent
    std::function<void(juce::Uuid, int, bool)> onEffectBypassChanged;
    std::function<void(juce::Uuid, int, int, float)> onEffectParameterChanged;
    std::function<void(juce::Uuid, int)> onEffectRemoved;
    std::function<void(juce::Uuid, int)> onEffectAdded;

private:
    void updateEffectSlots();
    void addEffectFromCombo();
    void removeEffect(int index);

    // VST3 support
    void rebuildPluginComboItems();
    void addVST3Plugin(const juce::PluginDescription& desc);
    void scanForPlugins();

    AudioTrack* currentTrack = nullptr;

    // Header controls
    juce::Label titleLabel { {}, "Effects" };
    juce::ComboBox effectTypeCombo;
    juce::TextButton addButton { "Add" };
    juce::TextButton scanButton { "Scan" };

    // Scrollable container
    juce::Viewport viewport;
    juce::Component effectContainer;
    std::vector<std::unique_ptr<EffectSlotComponent>> effectSlots;

    // Effect type IDs for combo
    enum EffectType
    {
        None = 0,
        Gain = 1,
        EQ = 2,
        Compressor = 3,
        Reverb = 4,
        Delay = 5,
        Oscillator = 6,
        Envelope = 7,
        Kick = 8,
        Filter = 9,
        Synth = 10,
        Tremolo = 11,
        Spatializer = 12,
        // VST3 plugins start at 1000
        VST3Base = 1000
    };

    // Cache of plugin descriptions (for combo items)
    juce::Array<juce::PluginDescription> pluginDescriptions;

    // Tempo for delay sync
    double currentTempo = 120.0;

    // Preset system
    std::unique_ptr<PresetManager> presetManager;
    void showPresetBrowser(int effectIndex);

    // Styling constants
    static constexpr int headerHeight = 36;
    static constexpr int slotSpacing = 6;
    static constexpr int padding = 8;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectRackPanel)
};
