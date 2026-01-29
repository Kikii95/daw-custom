#include "PresetBrowser.h"

PresetBrowser::PresetBrowser(PresetManager& manager)
    : presetManager(manager)
{
    // Title
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Category filter
    categoryLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(categoryLabel);

    categoryFilter.addItem("All", 1);
    categoryFilter.setSelectedId(1);
    categoryFilter.onChange = [this]()
    {
        auto idx = categoryFilter.getSelectedId();
        if (idx == 1)
            selectedCategory = "All";
        else
            selectedCategory = categoryFilter.getItemText(categoryFilter.getSelectedItemIndex());
        filterByCategory();
    };
    addAndMakeVisible(categoryFilter);

    // Preset list
    presetList.setRowHeight(28);
    presetList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff2a2a2a));
    addAndMakeVisible(presetList);

    // Buttons
    saveButton.onClick = [this]()
    {
        if (onSaveRequested)
            onSaveRequested();
    };
    addAndMakeVisible(saveButton);

    closeButton.onClick = [this]()
    {
        if (onCloseRequested)
            onCloseRequested();
    };
    addAndMakeVisible(closeButton);
}

void PresetBrowser::setEffectType(const juce::String& type)
{
    currentEffectType = type;
    titleLabel.setText(type + " Presets", juce::dontSendNotification);
    refresh();
}

void PresetBrowser::refresh()
{
    // Get all presets for this effect type
    currentPresets = presetManager.getPresetsForEffect(currentEffectType);
    updateCategoryFilter();
    filterByCategory();
}

void PresetBrowser::updateCategoryFilter()
{
    categoryFilter.clear(juce::dontSendNotification);
    categoryFilter.addItem("All", 1);

    juce::StringArray categories;
    for (const auto& preset : currentPresets)
    {
        if (preset.category.isNotEmpty() && !categories.contains(preset.category))
            categories.add(preset.category);
    }

    categories.sort(false);
    int id = 2;
    for (const auto& cat : categories)
        categoryFilter.addItem(cat, id++);

    categoryFilter.setSelectedId(1);
}

void PresetBrowser::filterByCategory()
{
    if (selectedCategory == "All")
    {
        currentPresets = presetManager.getPresetsForEffect(currentEffectType);
    }
    else
    {
        auto allPresets = presetManager.getPresetsForEffect(currentEffectType);
        currentPresets.clear();

        for (const auto& preset : allPresets)
        {
            if (preset.category == selectedCategory)
                currentPresets.add(preset);
        }
    }

    presetList.updateContent();
    presetList.repaint();
}

void PresetBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));

    // Border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRect(getLocalBounds(), 1);

    // Header background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(0, 0, getWidth(), 35);
}

void PresetBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(8);

    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(30));

    // Category filter
    auto filterArea = bounds.removeFromTop(30);
    categoryLabel.setBounds(filterArea.removeFromLeft(70));
    filterArea.removeFromLeft(5);
    categoryFilter.setBounds(filterArea);

    bounds.removeFromTop(5);

    // Buttons at bottom
    auto buttonArea = bounds.removeFromBottom(30);
    closeButton.setBounds(buttonArea.removeFromRight(70));
    buttonArea.removeFromRight(5);
    saveButton.setBounds(buttonArea.removeFromRight(70));

    bounds.removeFromBottom(5);

    // List takes the rest
    presetList.setBounds(bounds);
}

int PresetBrowser::getNumRows()
{
    return currentPresets.size();
}

void PresetBrowser::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < 0 || row >= currentPresets.size())
        return;

    const auto& preset = currentPresets[row];

    // Background
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff4a90d9));
    else if (row % 2 == 0)
        g.fillAll(juce::Colour(0xff2a2a2a));
    else
        g.fillAll(juce::Colour(0xff252525));

    // Text color
    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colour(0xffcccccc));

    // Preset name
    g.setFont(14.0f);
    g.drawText(preset.name, 10, 0, width - 100, height, juce::Justification::centredLeft);

    // Category (lighter color)
    g.setColour(rowIsSelected ? juce::Colours::white.withAlpha(0.7f) : juce::Colour(0xff808080));
    g.setFont(12.0f);
    g.drawText(preset.category, width - 90, 0, 80, height, juce::Justification::centredRight);
}

void PresetBrowser::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < currentPresets.size())
    {
        if (onPresetSelected)
            onPresetSelected(currentPresets[row]);
    }
}

void PresetBrowser::selectedRowsChanged(int /*lastRowSelected*/)
{
    // Could show preview or info here
}
