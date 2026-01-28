#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Audio/AudioEngine.h"
#include "Audio/AudioMixer.h"
#include "Audio/TransportController.h"
#include "Audio/FileIO/AudioFileLoader.h"
#include "Audio/FileIO/AudioFileExporter.h"
#include "Model/Project.h"
#include "UI/MainLayout.h"
#include "UI/Timeline/TimelinePanel.h"
#include "UI/Mixer/MixerPanel.h"
#include <memory>

class MainComponent : public juce::Component,
                      public juce::FileDragAndDropTarget,
                      public juce::MenuBarModel
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Drag & drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Menu bar
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void importAudioFile();
    void exportMix();
    void addTrack();

    void connectCallbacks();
    void updateTitle();

    // Core systems
    AudioEngine audioEngine;
    TransportController transport;
    std::unique_ptr<AudioMixer> mixer;
    AudioFileLoader fileLoader;
    AudioFileExporter fileExporter;

    // Project
    std::unique_ptr<Project> project;

    // UI
    juce::MenuBarComponent menuBar;
    MainLayout mainLayout;
    TimelinePanel timelinePanel;
    MixerPanel mixerPanel;

    // Menu IDs
    enum MenuIDs
    {
        NewProject = 1,
        OpenProject,
        SaveProject,
        SaveProjectAs,
        ImportAudio,
        ExportMix,
        Quit,
        AddTrack,
        DeleteTrack,
        ZoomIn,
        ZoomOut
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
