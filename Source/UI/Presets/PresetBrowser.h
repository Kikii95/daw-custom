#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Presets/PresetManager.h"

/**
 * Popup browser for selecting and managing effect presets.
 *
 * Features:
 * - List of presets for current effect type
 * - Category filtering
 * - Double-click to apply preset
 * - Save current settings as new preset
 */
class PresetBrowser : public juce::Component,
                      public juce::ListBoxModel
{
public:
    explicit PresetBrowser(PresetManager& manager);
    ~PresetBrowser() override = default;

    // Set the effect type to show presets for
    void setEffectType(const juce::String& type);
    juce::String getEffectType() const { return currentEffectType; }

    // Refresh preset list
    void refresh();

    // Callbacks
    std::function<void(const Preset&)> onPresetSelected;
    std::function<void()> onSaveRequested;
    std::function<void()> onCloseRequested;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
    void selectedRowsChanged(int lastRowSelected) override;

private:
    PresetManager& presetManager;
    juce::String currentEffectType;
    juce::Array<Preset> currentPresets;

    // UI Components
    juce::Label titleLabel { {}, "Presets" };
    juce::ListBox presetList { "Presets", this };
    juce::TextButton saveButton { "Save..." };
    juce::TextButton closeButton { "Close" };
    juce::ComboBox categoryFilter;
    juce::Label categoryLabel { {}, "Category:" };

    // Category filtering
    juce::String selectedCategory { "All" };
    void filterByCategory();
    void updateCategoryFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};
