#include "PluginBrowser.h"

//=============================================================================
// PluginListModel
//=============================================================================
int PluginBrowser::PluginListModel::getNumRows()
{
    return static_cast<int>(filteredPlugins.size());
}

void PluginBrowser::PluginListModel::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                                       int width, int height,
                                                       bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(filteredPlugins.size()))
        return;

    const auto& plugin = filteredPlugins[static_cast<size_t>(rowNumber)];

    // Background
    if (rowIsSelected)
    {
        g.setColour(Theme::Colours::accent());
        g.fillRect(0, 0, width, height);
        g.setColour(Theme::colour(Theme::textOnAccent));
    }
    else
    {
        g.setColour((rowNumber % 2 == 0) ? Theme::colour(Theme::bgSlot)
                                          : Theme::colour(Theme::bgPanel));
        g.fillRect(0, 0, width, height);
        g.setColour(Theme::Colours::text());
    }

    // Plugin name
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(plugin.name, 10, 2, width - 20, 20, juce::Justification::centredLeft, true);

    // Plugin details (manufacturer, category)
    g.setFont(juce::Font(11.0f));
    g.setColour(rowIsSelected ? Theme::colour(Theme::textOnAccent).withAlpha(0.8f)
                               : Theme::Colours::textMuted());
    juce::String details = plugin.manufacturerName;
    if (plugin.category.isNotEmpty())
        details += " - " + plugin.category;
    g.drawText(details, 10, 22, width - 20, 16, juce::Justification::centredLeft, true);
}

void PluginBrowser::PluginListModel::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < static_cast<int>(filteredPlugins.size()))
    {
        // Single click - could show details
    }
}

void PluginBrowser::PluginListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < static_cast<int>(filteredPlugins.size()))
    {
        if (browser.onPluginSelected)
            browser.onPluginSelected(filteredPlugins[static_cast<size_t>(row)]);
    }
}

void PluginBrowser::PluginListModel::setFilteredPlugins(const std::vector<juce::PluginDescription>& plugins)
{
    filteredPlugins = plugins;
}

const juce::PluginDescription* PluginBrowser::PluginListModel::getPluginAt(int index) const
{
    if (index >= 0 && index < static_cast<int>(filteredPlugins.size()))
        return &filteredPlugins[static_cast<size_t>(index)];
    return nullptr;
}

//=============================================================================
// PluginBrowser
//=============================================================================
PluginBrowser::PluginBrowser()
{
    // Search box
    searchBox.setTextToShowWhenEmpty("Search plugins...", Theme::Colours::textMuted());
    searchBox.setColour(juce::TextEditor::backgroundColourId, Theme::colour(Theme::bgSlot));
    searchBox.setColour(juce::TextEditor::textColourId, Theme::Colours::text());
    searchBox.setColour(juce::TextEditor::outlineColourId, Theme::colour(Theme::border));
    searchBox.addListener(this);
    addAndMakeVisible(searchBox);

    // Scan button
    scanButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::accent());
    scanButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    scanButton.onClick = [this]()
    {
        if (onScanRequested)
            onScanRequested();
    };
    addAndMakeVisible(scanButton);

    // List box
    listModel = std::make_unique<PluginListModel>(*this);
    pluginListBox.setModel(listModel.get());
    pluginListBox.setRowHeight(44);
    pluginListBox.setColour(juce::ListBox::backgroundColourId, Theme::colour(Theme::bgPanel));
    pluginListBox.setColour(juce::ListBox::outlineColourId, Theme::colour(Theme::border));
    addAndMakeVisible(pluginListBox);
}

void PluginBrowser::paint(juce::Graphics& g)
{
    g.fillAll(Theme::colour(Theme::bgPanel));
}

void PluginBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(Theme::spacing_sm);

    // Top row: search + scan button
    auto topRow = bounds.removeFromTop(32);
    scanButton.setBounds(topRow.removeFromRight(100));
    topRow.removeFromRight(Theme::spacing_sm);
    searchBox.setBounds(topRow);

    bounds.removeFromTop(Theme::spacing_sm);

    // Plugin list
    pluginListBox.setBounds(bounds);
}

void PluginBrowser::setPluginList(const juce::KnownPluginList& list)
{
    pluginList = &list;
    refresh();
}

void PluginBrowser::refresh()
{
    buildCategories();
    filterPlugins(searchBox.getText());
}

void PluginBrowser::textEditorTextChanged(juce::TextEditor& editor)
{
    if (&editor == &searchBox)
    {
        filterPlugins(searchBox.getText());
    }
}

void PluginBrowser::buildCategories()
{
    categories.clear();

    if (!pluginList)
        return;

    std::map<juce::String, std::vector<juce::PluginDescription>> categoryMap;

    for (const auto& type : pluginList->getTypes())
    {
        juce::String cat = type.category.isEmpty() ? "Uncategorized" : type.category;
        categoryMap[cat].push_back(type);
    }

    for (auto& [catName, plugins] : categoryMap)
    {
        PluginCategory cat;
        cat.name = catName;
        cat.plugins = std::move(plugins);
        categories.push_back(std::move(cat));
    }

    // Sort categories alphabetically
    std::sort(categories.begin(), categories.end(),
              [](const PluginCategory& a, const PluginCategory& b)
              {
                  return a.name.compareIgnoreCase(b.name) < 0;
              });
}

void PluginBrowser::filterPlugins(const juce::String& searchText)
{
    std::vector<juce::PluginDescription> filtered;

    if (!pluginList)
    {
        listModel->setFilteredPlugins(filtered);
        pluginListBox.updateContent();
        return;
    }

    juce::String search = searchText.toLowerCase();

    for (const auto& type : pluginList->getTypes())
    {
        if (search.isEmpty() ||
            type.name.toLowerCase().contains(search) ||
            type.manufacturerName.toLowerCase().contains(search) ||
            type.category.toLowerCase().contains(search))
        {
            filtered.push_back(type);
        }
    }

    // Sort by name
    std::sort(filtered.begin(), filtered.end(),
              [](const juce::PluginDescription& a, const juce::PluginDescription& b)
              {
                  return a.name.compareIgnoreCase(b.name) < 0;
              });

    listModel->setFilteredPlugins(filtered);
    pluginListBox.updateContent();
    pluginListBox.repaint();
}
