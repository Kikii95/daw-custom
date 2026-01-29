#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "UI/Theme/AppTheme.h"
#include <functional>

/**
 * A searchable browser for VST3 plugins.
 * Groups plugins by category with a search filter.
 */
class PluginBrowser : public juce::Component,
                      public juce::TextEditor::Listener
{
public:
    PluginBrowser();
    ~PluginBrowser() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the plugin list to display
    void setPluginList(const juce::KnownPluginList& list);

    // Refresh from current plugin list
    void refresh();

    // Callbacks
    std::function<void(const juce::PluginDescription&)> onPluginSelected;
    std::function<void()> onScanRequested;

    // TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor& editor) override;

private:
    struct PluginCategory
    {
        juce::String name;
        std::vector<juce::PluginDescription> plugins;
    };

    std::vector<PluginCategory> categories;
    const juce::KnownPluginList* pluginList = nullptr;

    // UI Components
    juce::TextEditor searchBox;
    juce::TextButton scanButton { "Scan Plugins" };
    juce::ListBox pluginListBox;

    class PluginListModel : public juce::ListBoxModel
    {
    public:
        PluginListModel(PluginBrowser& owner) : browser(owner) {}

        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g,
                              int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;

        void setFilteredPlugins(const std::vector<juce::PluginDescription>& plugins);
        const juce::PluginDescription* getPluginAt(int index) const;

    private:
        PluginBrowser& browser;
        std::vector<juce::PluginDescription> filteredPlugins;
    };

    std::unique_ptr<PluginListModel> listModel;

    void buildCategories();
    void filterPlugins(const juce::String& searchText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBrowser)
};
