#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Audio/DSP/EffectSlot.h"
#include "Audio/Plugins/VST3EffectSlot.h"
#include <functional>
#include <vector>
#include <memory>

/**
 * UI component for a single effect slot.
 * Displays effect name, bypass toggle, remove button, and dynamic parameter controls.
 * Supports drag & drop for reordering effects.
 */
class EffectSlotComponent : public juce::Component
{
public:
    EffectSlotComponent();
    ~EffectSlotComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Mouse handling for drag reorder
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // Bind to an effect
    void setEffect(EffectSlot* effect, int index);
    EffectSlot* getEffect() const { return effectSlot; }
    int getEffectIndex() const { return effectIndex; }

    // Callbacks to parent
    std::function<void(int, bool)> onBypassChanged;
    std::function<void(int, int, float)> onParameterChanged;
    std::function<void(int)> onRemoveClicked;
    std::function<void(int)> onEditClicked;  // For VST3 plugin UI
    std::function<void(int)> onPresetClicked;  // Open preset browser
    std::function<void(int, int)> onReorder;  // (fromIndex, toIndex) for drag reorder

    // Tempo sync for delay (optional)
    void setTempo(double bpm);

    // Height calculation for layout
    int getPreferredHeight() const;

private:
    void buildParameterControls();
    void clearParameterControls();

    EffectSlot* effectSlot = nullptr;
    int effectIndex = 0;

    // Header controls
    juce::Label nameLabel;
    juce::TextButton bypassButton { "BYP" };
    juce::TextButton presetButton { "P" };  // Open preset browser
    juce::TextButton editButton { "Edit" };  // Shows only for VST3 plugins
    juce::TextButton removeButton { "X" };

    bool isVST3Effect = false;

    // Dynamic parameter controls
    struct ParamControl
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
    };
    std::vector<ParamControl> paramControls;

    // Tempo sync (for Delay effect)
    juce::ToggleButton syncButton { "Sync" };
    juce::ComboBox noteValueCombo;
    bool isDelayEffect = false;
    double currentTempo = 120.0;

    // Drag state
    bool dragging = false;
    juce::Point<int> dragStartPos;
    int dragStartY = 0;

    // Styling constants
    static constexpr int headerHeight = 32;
    static constexpr int paramRowHeight = 50;
    static constexpr int padding = 4;
    static constexpr int dragHandleWidth = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectSlotComponent)
};
