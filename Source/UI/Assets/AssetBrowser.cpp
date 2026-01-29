#include "AssetBrowser.h"
#include "UI/Theme/AppTheme.h"

AssetBrowser::AssetBrowser()
{
    // Title
    titleLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // Bookmarks combo
    bookmarksCombo.setColour(juce::ComboBox::backgroundColourId, Theme::colour(Theme::bgHover));
    bookmarksCombo.setColour(juce::ComboBox::textColourId, Theme::Colours::text());
    bookmarksCombo.setTooltip("Quick access folders");
    bookmarksCombo.onChange = [this]()
    {
        int idx = bookmarksCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < static_cast<int>(bookmarks.size()))
            navigateToFolder(bookmarks[static_cast<size_t>(idx)].folder);
    };
    addAndMakeVisible(bookmarksCombo);

    // Refresh button
    refreshButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgHover));
    refreshButton.setColour(juce::TextButton::textColourOffId, Theme::Colours::text());
    refreshButton.setTooltip("Refresh file list");
    refreshButton.onClick = [this]()
    {
        if (directoryList)
            directoryList->refresh();
    };
    addAndMakeVisible(refreshButton);

    // Recent files section
    recentLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    recentLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(recentLabel);

    recentFilesModel.owner = this;
    recentFilesList.setModel(&recentFilesModel);
    recentFilesList.setColour(juce::ListBox::backgroundColourId, Theme::colour(Theme::bgSlot));
    recentFilesList.setColour(juce::ListBox::outlineColourId, Theme::colour(Theme::border));
    recentFilesList.setRowHeight(24);
    recentFilesList.setMultipleSelectionEnabled(false);
    addAndMakeVisible(recentFilesList);

    // Setup bookmarks
    setupBookmarks();

    // Start scan thread
    scanThread.startThread(juce::Thread::Priority::low);
}

AssetBrowser::~AssetBrowser()
{
    fileTree.reset();
    directoryList.reset();
    scanThread.stopThread(1000);
}

void AssetBrowser::paint(juce::Graphics& g)
{
    g.fillAll(Theme::colour(Theme::bgPanel));

    // Header background
    auto headerBounds = getLocalBounds().removeFromTop(headerHeight);
    g.setColour(Theme::colour(Theme::bgHeader));
    g.fillRect(headerBounds);

    // Separator after header
    g.setColour(Theme::colour(Theme::border));
    g.fillRect(0, headerHeight, getWidth(), 1);

    // Separator before recent files
    auto recentY = getHeight() - recentSectionHeight;
    g.fillRect(0, recentY, getWidth(), 1);
}

void AssetBrowser::resized()
{
    auto bounds = getLocalBounds();

    // Header
    auto header = bounds.removeFromTop(headerHeight).reduced(padding, 4);
    titleLabel.setBounds(header.removeFromLeft(50));
    header.removeFromLeft(4);
    refreshButton.setBounds(header.removeFromRight(60));
    header.removeFromRight(4);
    bookmarksCombo.setBounds(header);

    bounds.removeFromTop(1); // separator

    // Recent files at bottom
    auto recentBounds = bounds.removeFromBottom(recentSectionHeight);
    recentBounds.removeFromTop(1); // separator
    recentLabel.setBounds(recentBounds.removeFromTop(20).reduced(padding, 2));
    recentFilesList.setBounds(recentBounds.reduced(padding));

    // File tree fills the rest
    if (fileTree)
        fileTree->setBounds(bounds.reduced(padding));
}

void AssetBrowser::setFormatManager(juce::AudioFormatManager* manager)
{
    formatManager = manager;

    // Create directory list with audio file filter
    auto filter = std::make_unique<juce::WildcardFileFilter>(
        getWildcardFilter(), "*", "Audio Files");

    directoryList = std::make_unique<juce::DirectoryContentsList>(filter.release(), scanThread);
    directoryList->setDirectory(juce::File::getSpecialLocation(juce::File::userMusicDirectory), true, true);

    // Create file tree
    fileTree = std::make_unique<juce::FileTreeComponent>(*directoryList);
    fileTree->setColour(juce::FileTreeComponent::backgroundColourId, Theme::colour(Theme::bgSlot));
    fileTree->setColour(juce::FileTreeComponent::linesColourId, Theme::colour(Theme::border));
    fileTree->addListener(this);
    fileTree->setDragAndDropDescription("AudioFile");
    addAndMakeVisible(*fileTree);

    resized();
}

void AssetBrowser::addRecentFile(const juce::File& file)
{
    // Remove if already exists
    auto it = std::find(recentFilesModel.files.begin(), recentFilesModel.files.end(), file);
    if (it != recentFilesModel.files.end())
        recentFilesModel.files.erase(it);

    // Add to front
    recentFilesModel.files.insert(recentFilesModel.files.begin(), file);

    // Keep max 10 recent files
    if (recentFilesModel.files.size() > 10)
        recentFilesModel.files.resize(10);

    recentFilesList.updateContent();
}

void AssetBrowser::clearRecentFiles()
{
    recentFilesModel.files.clear();
    recentFilesList.updateContent();
}

void AssetBrowser::selectionChanged()
{
    // Nothing special needed
}

void AssetBrowser::fileClicked(const juce::File& file, const juce::MouseEvent& /*e*/)
{
    if (file.existsAsFile())
    {
        // Start drag if it's an audio file
        if (onFileDragStarted)
            onFileDragStarted(file);
    }
}

void AssetBrowser::fileDoubleClicked(const juce::File& file)
{
    if (file.existsAsFile())
    {
        addRecentFile(file);

        if (onFileDoubleClicked)
            onFileDoubleClicked(file);
    }
    else if (file.isDirectory())
    {
        navigateToFolder(file);
    }
}

void AssetBrowser::browserRootChanged(const juce::File& /*newRoot*/)
{
    // Update bookmarks combo if needed
}

void AssetBrowser::setupBookmarks()
{
    bookmarks.clear();
    bookmarksCombo.clear();

    // Standard folders
    bookmarks.push_back({ "Music", juce::File::getSpecialLocation(juce::File::userMusicDirectory) });
    bookmarks.push_back({ "Downloads", juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("Downloads") });
    bookmarks.push_back({ "Desktop", juce::File::getSpecialLocation(juce::File::userDesktopDirectory) });
    bookmarks.push_back({ "Home", juce::File::getSpecialLocation(juce::File::userHomeDirectory) });

    // Add to combo
    for (size_t i = 0; i < bookmarks.size(); ++i)
    {
        bookmarksCombo.addItem(bookmarks[i].name, static_cast<int>(i + 1));
    }

    bookmarksCombo.setSelectedId(1, juce::dontSendNotification);
}

void AssetBrowser::navigateToFolder(const juce::File& folder)
{
    if (directoryList && folder.isDirectory())
    {
        directoryList->setDirectory(folder, true, true);
    }
}

juce::String AssetBrowser::getWildcardFilter() const
{
    if (formatManager == nullptr)
        return "*.wav;*.mp3;*.flac;*.ogg;*.aiff";

    juce::String wildcards;
    for (int i = 0; i < formatManager->getNumKnownFormats(); ++i)
    {
        auto* format = formatManager->getKnownFormat(i);
        auto extensions = format->getFileExtensions();
        for (const auto& ext : extensions)
        {
            if (wildcards.isNotEmpty())
                wildcards += ";";
            wildcards += "*" + ext;
        }
    }
    return wildcards;
}

void AssetBrowser::refreshRecentFilesList()
{
    recentFilesList.updateContent();
}

// RecentFilesModel implementation
void AssetBrowser::RecentFilesModel::paintListBoxItem(int row, juce::Graphics& g,
                                                       int width, int height, bool selected)
{
    if (row < 0 || row >= static_cast<int>(files.size()))
        return;

    auto bounds = juce::Rectangle<int>(0, 0, width, height);

    // Selection background
    if (selected)
    {
        g.setColour(Theme::Colours::accent().withAlpha(0.3f));
        g.fillRect(bounds);
    }

    // File icon
    g.setColour(Theme::Colours::textMuted());
    g.setFont(12.0f);
    g.drawText(juce::CharPointer_UTF8("\xf0\x9f\x8e\xb5"), bounds.removeFromLeft(24),
               juce::Justification::centred);

    // File name
    g.setColour(Theme::Colours::text());
    g.drawText(files[static_cast<size_t>(row)].getFileName(),
               bounds.reduced(4, 0),
               juce::Justification::centredLeft, true);
}

juce::var AssetBrowser::RecentFilesModel::getDragSourceDescription(const juce::SparseSet<int>& selectedRows)
{
    if (selectedRows.isEmpty())
        return {};

    int row = selectedRows[0];
    if (row >= 0 && row < static_cast<int>(files.size()))
    {
        return files[static_cast<size_t>(row)].getFullPathName();
    }
    return {};
}
