#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <functional>
#include <vector>

/**
 * Asset Browser panel for browsing and importing audio files.
 * Similar to Unity's Project panel - shows file tree + recent files.
 */
class AssetBrowser : public juce::Component,
                     public juce::FileBrowserListener,
                     public juce::DragAndDropContainer
{
public:
    AssetBrowser();
    ~AssetBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the format manager for file type filtering
    void setFormatManager(juce::AudioFormatManager* manager);

    // Add a file to recent files list
    void addRecentFile(const juce::File& file);

    // Clear recent files
    void clearRecentFiles();

    // FileBrowserListener
    void selectionChanged() override;
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;

    // Callbacks
    std::function<void(const juce::File&)> onFileDoubleClicked;
    std::function<void(const juce::File&)> onFileDragStarted;

private:
    void setupBookmarks();
    void navigateToFolder(const juce::File& folder);
    void refreshRecentFilesList();
    juce::String getWildcardFilter() const;

    // File tree
    juce::TimeSliceThread scanThread { "Asset Scanner" };
    std::unique_ptr<juce::DirectoryContentsList> directoryList;
    std::unique_ptr<juce::FileTreeComponent> fileTree;

    // UI Components
    juce::Label titleLabel { {}, "Assets" };
    juce::ComboBox bookmarksCombo;
    juce::TextButton refreshButton { "Refresh" };

    // Recent files section
    juce::Label recentLabel { {}, "Recent Files" };
    juce::ListBox recentFilesList;

    // Recent files data
    class RecentFilesModel : public juce::ListBoxModel
    {
    public:
        int getNumRows() override { return static_cast<int>(files.size()); }
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
        juce::var getDragSourceDescription(const juce::SparseSet<int>& selectedRows) override;

        std::vector<juce::File> files;
        AssetBrowser* owner = nullptr;
    };
    RecentFilesModel recentFilesModel;

    // Bookmarks
    struct Bookmark
    {
        juce::String name;
        juce::File folder;
    };
    std::vector<Bookmark> bookmarks;

    juce::AudioFormatManager* formatManager = nullptr;

    // Layout constants
    static constexpr int headerHeight = 32;
    static constexpr int recentSectionHeight = 120;
    static constexpr int padding = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AssetBrowser)
};
