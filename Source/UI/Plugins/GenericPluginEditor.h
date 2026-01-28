#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class VST3EffectSlot;

/**
 * Generic fallback editor for VST3 plugins without a native UI.
 *
 * Displays a grid of sliders for all automatable parameters.
 * Used when plugin->hasEditor() returns false.
 */
class GenericPluginEditor : public juce::Component,
                           private juce::Timer
{
public:
    explicit GenericPluginEditor(VST3EffectSlot* effectSlot);
    ~GenericPluginEditor() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Preferred size
    int getPreferredWidth() const { return preferredWidth; }
    int getPreferredHeight() const { return preferredHeight; }

private:
    VST3EffectSlot* slot = nullptr;

    // Parameter controls
    struct ParameterControl
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        int paramIndex = 0;
    };

    juce::OwnedArray<ParameterControl> controls;

    // Layout
    static constexpr int PARAM_WIDTH = 80;
    static constexpr int PARAM_HEIGHT = 100;
    static constexpr int COLUMNS = 4;
    static constexpr int PADDING = 10;

    int preferredWidth = 400;
    int preferredHeight = 300;

    // Timer for parameter polling (some plugins don't notify)
    void timerCallback() override;

    void buildControls();
    void updateControlValues();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericPluginEditor)
};
