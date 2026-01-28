#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class VST3EffectSlot;

/**
 * Window hosting a VST3 plugin's native editor UI.
 *
 * - Wraps the plugin's AudioProcessorEditor
 * - Handles window sizing and constraints
 * - Cleans up editor when closed
 */
class PluginEditorWindow : public juce::DocumentWindow
{
public:
    PluginEditorWindow(VST3EffectSlot* effectSlot);
    ~PluginEditorWindow() override;

    // DocumentWindow overrides
    void closeButtonPressed() override;

    // Get the plugin slot this window is editing
    VST3EffectSlot* getEffectSlot() const { return slot; }

    // Check if plugin is still valid
    bool isPluginValid() const;

    // Static factory - finds existing window or creates new one
    static PluginEditorWindow* getOrCreateWindow(VST3EffectSlot* slot);
    static void closeWindowFor(VST3EffectSlot* slot);
    static void closeAllWindows();

private:
    VST3EffectSlot* slot = nullptr;
    std::unique_ptr<juce::AudioProcessorEditor> editor;

    // Track all open windows
    static juce::Array<PluginEditorWindow*> openWindows;

    void createEditor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditorWindow)
};
