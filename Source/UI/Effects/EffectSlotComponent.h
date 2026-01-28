#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Audio/DSP/EffectSlot.h"
#include <functional>
#include <vector>
#include <memory>

/**
 * UI component for a single effect slot.
 * Displays effect name, bypass toggle, remove button, and dynamic parameter controls.
 */
class EffectSlotComponent : public juce::Component
{
public:
    EffectSlotComponent();
    ~EffectSlotComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Bind to an effect
    void setEffect(EffectSlot* effect, int index);
    EffectSlot* getEffect() const { return effectSlot; }
    int getEffectIndex() const { return effectIndex; }

    // Callbacks to parent
    std::function<void(int, bool)> onBypassChanged;
    std::function<void(int, int, float)> onParameterChanged;
    std::function<void(int)> onRemoveClicked;

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
    juce::TextButton removeButton { "X" };

    // Dynamic parameter controls
    struct ParamControl
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
    };
    std::vector<ParamControl> paramControls;

    // Styling constants
    static constexpr int headerHeight = 32;
    static constexpr int paramRowHeight = 50;
    static constexpr int padding = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectSlotComponent)
};
