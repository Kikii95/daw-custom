#include "MainComponent.h"
#include "Audio/AudioClip.h"
#include "Audio/Plugins/PluginManager.h"
#include "UI/Plugins/PluginEditorWindow.h"
#include "Model/ProjectSerializer.h"

MainComponent::MainComponent()
{
    // Apply custom look and feel
    setLookAndFeel(&modernLookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel(&modernLookAndFeel);

    // Initialize plugin manager (singleton)
    PluginManager::getInstance().initialize();

    // Create project and mixer
    project = std::make_unique<Project>();
    mixer = std::make_unique<AudioMixer>();

    // Initialize audio engine
    audioEngine.initialise(0, 2);
    audioEngine.setSource(mixer.get());

    // Connect mixer to transport
    mixer->setTransportController(&transport);

    // Set up menu bar
    menuBar.setModel(this);
    addAndMakeVisible(menuBar);

    // Set up main layout
    mainLayout.getTransportBar().setTransportController(&transport);
    addAndMakeVisible(mainLayout);

    // Set up timeline panel
    timelinePanel.setProject(project.get());
    timelinePanel.setTransportController(&transport);
    timelinePanel.setFormatManager(&fileLoader.getFormatManager());
    addAndMakeVisible(timelinePanel);

    // Set up mixer panel
    mixerPanel.setProject(project.get());
    mixerPanel.setAudioMixer(mixer.get());
    addAndMakeVisible(mixerPanel);

    // Set up effect rack panel
    addAndMakeVisible(effectRackPanel);

    // Set up asset browser
    assetBrowser.setFormatManager(&fileLoader.getFormatManager());
    addAndMakeVisible(assetBrowser);

    // Connect callbacks
    connectCallbacks();

    // Create default track
    addTrack();

    // Set default size
    setSize(1400, 900);

    // Register keyboard listener
    addKeyListener(this);
    setWantsKeyboardFocus(true);

    updateTitle();
}

MainComponent::~MainComponent()
{
    // Clear look and feel before destruction
    setLookAndFeel(nullptr);

    // Close all plugin editor windows
    PluginEditorWindow::closeAllWindows();

    audioEngine.clearSource();
    audioEngine.shutdown();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Menu bar at top
    menuBar.setBounds(bounds.removeFromTop(24));

    // Transport bar at top
    auto transportHeight = 60;
    auto transportArea = bounds.removeFromTop(transportHeight);
    mainLayout.setBounds(transportArea);  // Give mainLayout bounds so transportBar can display

    // Effect rack on right
    auto effectRackWidth = 250;
    effectRackPanel.setBounds(bounds.removeFromRight(effectRackWidth));

    // Asset browser on left
    auto assetBrowserWidth = 200;
    assetBrowser.setBounds(bounds.removeFromLeft(assetBrowserWidth));

    // Mixer at bottom
    auto mixerHeight = 200;
    mixerPanel.setBounds(bounds.removeFromBottom(mixerHeight));

    // Timeline takes the rest
    timelinePanel.setBounds(bounds);
}

bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (fileLoader.isFormatSupported(juce::File(file)))
            return true;
    }
    return false;
}

void MainComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (const auto& filePath : files)
    {
        juce::File file(filePath);

        if (!fileLoader.isFormatSupported(file))
            continue;

        // Load the audio file
        auto result = fileLoader.loadFile(file);

        if (!result.isValid())
        {
            DBG("Failed to load: " + result.error);
            continue;
        }

        // Add to first track (or selected track)
        if (project->getNumTracks() == 0)
            addTrack();

        auto* track = project->getTrackByIndex(0);
        if (track == nullptr)
            continue;

        // Create clip metadata
        Clip clip;
        clip.id = juce::Uuid();
        clip.name = file.getFileNameWithoutExtension();
        clip.sourceFile = file;
        clip.startTime = 0.0;
        clip.duration = result.lengthInSeconds;
        clip.sourceSampleRate = result.sampleRate;
        clip.colour = track->colour;

        // Add to project track (for UI/saving)
        track->addClip(clip);

        // Create AudioClip with actual audio data and add to mixer's AudioTrack
        if (auto* audioTrack = mixer->getTrack(0))
        {
            auto audioClip = std::make_unique<AudioClip>();
            audioClip->loadFromBuffer(std::move(result.buffer), result.sampleRate, clip);
            audioTrack->addClip(std::move(audioClip));
            audioTrack->setTrackData(*track);
        }

        // Update duration
        transport.setDuration(project->getTotalDuration());

        // Refresh UI
        timelinePanel.refreshTracks();
        mixerPanel.refreshChannels();

        project->setModified(true);
        updateTitle();
    }
}

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "File", "Edit", "View" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int menuIndex, const juce::String& /*menuName*/)
{
    juce::PopupMenu menu;

    switch (menuIndex)
    {
        case 0: // File
            menu.addItem(NewProject, "New Project", true, false);
            menu.addItem(OpenProject, "Open Project...", true, false);
            menu.addSeparator();
            menu.addItem(SaveProject, "Save", true, false);
            menu.addItem(SaveProjectAs, "Save As...", true, false);
            menu.addSeparator();
            menu.addItem(ImportAudio, "Import Audio...", true, false);
            menu.addItem(ExportMix, "Export Mix...", true, false);
            menu.addSeparator();
            menu.addItem(Quit, "Quit", true, false);
            break;

        case 1: // Edit
            menu.addItem(AddTrack, "Add Track", true, false);
            menu.addItem(DeleteTrack, "Delete Track", true, false);
            break;

        case 2: // View
            menu.addItem(ZoomIn, "Zoom In", true, false);
            menu.addItem(ZoomOut, "Zoom Out", true, false);
            break;
    }

    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
    switch (menuItemID)
    {
        case NewProject:     newProject(); break;
        case OpenProject:    openProject(); break;
        case SaveProject:    saveProject(); break;
        case SaveProjectAs:  saveProjectAs(); break;
        case ImportAudio:    importAudioFile(); break;
        case ExportMix:      exportMix(); break;
        case Quit:           juce::JUCEApplication::getInstance()->systemRequestedQuit(); break;
        case AddTrack:       addTrack(); break;
        case ZoomIn:         timelinePanel.zoomIn(); break;
        case ZoomOut:        timelinePanel.zoomOut(); break;
        default: break;
    }
}

void MainComponent::newProject()
{
    project = std::make_unique<Project>();
    mixer->clearTracks();

    timelinePanel.setProject(project.get());
    mixerPanel.setProject(project.get());

    transport.stop();
    transport.setDuration(0.0);

    addTrack();
    updateTitle();
}

void MainComponent::openProject()
{
    // Check for unsaved changes
    if (project->isModified())
    {
        auto result = juce::AlertWindow::showYesNoCancelBox(
            juce::MessageBoxIconType::QuestionIcon,
            "Unsaved Changes",
            "Current project has unsaved changes. Save before opening?",
            "Save", "Don't Save", "Cancel",
            nullptr, nullptr
        );

        if (result == 0) // Cancel
            return;

        if (result == 1) // Save
            saveProject();
    }

    auto chooser = std::make_shared<juce::FileChooser>(
        "Open Project",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*" + juce::String(ProjectSerializer::FILE_EXTENSION)
    );

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (!file.existsAsFile())
                return;

            // Clear current project
            project->clear();
            mixer->clearTracks();
            effectRackPanel.setTrack(nullptr);

            // Load from file
            if (ProjectSerializer::loadFromFile(file, *project, *mixer, fileLoader.getFormatManager()))
            {
                project->setProjectFile(file);

                // Update UI
                timelinePanel.setProject(project.get());
                mixerPanel.setProject(project.get());
                timelinePanel.refreshTracks();
                mixerPanel.refreshChannels();

                // Update transport duration
                transport.stop();
                transport.setPosition(0.0);
                transport.setDuration(project->getTotalDuration());

                // Auto-select first track
                if (mixer->getNumTracks() > 0)
                {
                    if (auto* audioTrack = mixer->getTrack(0))
                    {
                        effectRackPanel.setTrack(audioTrack);
                        mixerPanel.selectTrack(audioTrack->getId());
                    }
                }

                updateTitle();
                DBG("Project loaded: " + file.getFullPathName());
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Load Failed",
                    "Failed to load project: " + ProjectSerializer::getLastError()
                );
            }
        });
}

void MainComponent::saveProject()
{
    if (!project->hasBeenSaved())
    {
        saveProjectAs();
        return;
    }

    // Save to existing file
    if (ProjectSerializer::saveToFile(project->getProjectFile(), *project, *mixer))
    {
        project->setModified(false);
        updateTitle();
        DBG("Project saved: " + project->getProjectFile().getFullPathName());
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Save Failed",
            "Failed to save project: " + ProjectSerializer::getLastError()
        );
    }
}

void MainComponent::saveProjectAs()
{
    auto defaultFile = project->hasBeenSaved()
        ? project->getProjectFile()
        : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
              .getChildFile(project->getName() + ProjectSerializer::FILE_EXTENSION);

    auto chooser = std::make_shared<juce::FileChooser>(
        "Save Project As",
        defaultFile,
        "*" + juce::String(ProjectSerializer::FILE_EXTENSION)
    );

    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File())
                return;

            // Ensure correct extension
            if (!file.hasFileExtension(ProjectSerializer::FILE_EXTENSION))
                file = file.withFileExtension(ProjectSerializer::FILE_EXTENSION);

            // Update project name from filename
            project->setName(file.getFileNameWithoutExtension());

            if (ProjectSerializer::saveToFile(file, *project, *mixer))
            {
                project->setProjectFile(file);
                project->setModified(false);
                updateTitle();
                DBG("Project saved as: " + file.getFullPathName());
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Save Failed",
                    "Failed to save project: " + ProjectSerializer::getLastError()
                );
            }
        });
}

void MainComponent::importAudioFile()
{
    // Build wildcard pattern: "*.wav;*.mp3;*.flac;..."
    juce::String wildcards;
    for (const auto& ext : fileLoader.getSupportedExtensions())
    {
        if (wildcards.isNotEmpty())
            wildcards += ";";
        wildcards += "*" + ext;  // ext already has the dot (e.g. ".wav")
    }

    auto chooser = std::make_shared<juce::FileChooser>(
        "Import Audio File",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        wildcards
    );

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                filesDropped({ file.getFullPathName() }, 0, 0);
            }
        });
}

void MainComponent::exportMix()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Export Mix",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("export.wav"),
        "*.wav;*.flac"
    );

    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                AudioFileExporter::ExportSettings settings;
                settings.sampleRate = static_cast<int>(audioEngine.getSampleRate());
                settings.numChannels = 2;

                if (file.getFileExtension().equalsIgnoreCase(".flac"))
                    settings.format = AudioFileExporter::Format::FLAC;

                bool success = fileExporter.exportFromSource(
                    file,
                    *mixer,
                    project->getTotalDuration(),
                    settings,
                    [](double progress)
                    {
                        DBG("Export progress: " + juce::String(progress * 100, 1) + "%");
                    }
                );

                if (success)
                    DBG("Export completed: " + file.getFullPathName());
                else
                    DBG("Export failed: " + fileExporter.getLastError());
            }
        });
}

void MainComponent::addTrack()
{
    auto& track = project->addTrack();
    auto* audioTrack = mixer->addTrack();

    // Link AudioTrack to project Track data
    if (audioTrack != nullptr)
        audioTrack->setTrackData(track);

    timelinePanel.refreshTracks();
    mixerPanel.refreshChannels();

    // Auto-select first track so effect rack is populated
    if (project->getNumTracks() == 1 && audioTrack != nullptr)
    {
        mixerPanel.selectTrack(track.id);
        effectRackPanel.setTrack(audioTrack);
    }

    project->setModified(true);
    updateTitle();
}

void MainComponent::connectCallbacks()
{
    // Mixer panel callbacks
    mixerPanel.onTrackVolumeChanged = [this](juce::Uuid trackId, float volume)
    {
        if (auto* track = project->getTrack(trackId))
            track->volume = volume;

        // Update audio track
        for (int i = 0; i < mixer->getNumTracks(); ++i)
        {
            if (auto* audioTrack = mixer->getTrack(i))
            {
                if (audioTrack->getId() == trackId)
                    audioTrack->setVolume(volume);
            }
        }
    };

    mixerPanel.onTrackPanChanged = [this](juce::Uuid trackId, float pan)
    {
        if (auto* track = project->getTrack(trackId))
            track->pan = pan;

        for (int i = 0; i < mixer->getNumTracks(); ++i)
        {
            if (auto* audioTrack = mixer->getTrack(i))
            {
                if (audioTrack->getId() == trackId)
                    audioTrack->setPan(pan);
            }
        }
    };

    mixerPanel.onTrackMuteChanged = [this](juce::Uuid trackId, bool muted)
    {
        if (auto* track = project->getTrack(trackId))
            track->muted = muted;

        for (int i = 0; i < mixer->getNumTracks(); ++i)
        {
            if (auto* audioTrack = mixer->getTrack(i))
            {
                if (audioTrack->getId() == trackId)
                    audioTrack->setMuted(muted);
            }
        }
    };

    mixerPanel.onTrackSoloChanged = [this](juce::Uuid trackId, bool solo)
    {
        if (auto* track = project->getTrack(trackId))
            track->solo = solo;

        for (int i = 0; i < mixer->getNumTracks(); ++i)
        {
            if (auto* audioTrack = mixer->getTrack(i))
            {
                if (audioTrack->getId() == trackId)
                    audioTrack->setSolo(solo);
            }
        }
    };

    mixerPanel.onMasterVolumeChanged = [this](float volume)
    {
        mixer->setMasterVolume(volume);
        project->setMasterVolume(volume);
    };

    mixerPanel.onTrackSelected = [this](juce::Uuid trackId)
    {
        if (auto* audioTrack = getAudioTrackById(trackId))
            effectRackPanel.setTrack(audioTrack);
    };

    // Timeline track selection - sync with mixer and effect rack
    timelinePanel.onTrackSelected = [this](juce::Uuid trackId)
    {
        mixerPanel.selectTrack(trackId);
        if (auto* audioTrack = getAudioTrackById(trackId))
            effectRackPanel.setTrack(audioTrack);
    };

    // Effect rack callbacks
    effectRackPanel.onEffectAdded = [this](juce::Uuid /*trackId*/, int /*index*/)
    {
        project->setModified(true);
        updateTitle();
    };

    effectRackPanel.onEffectRemoved = [this](juce::Uuid /*trackId*/, int /*index*/)
    {
        project->setModified(true);
        updateTitle();
    };

    effectRackPanel.onEffectBypassChanged = [this](juce::Uuid /*trackId*/, int /*index*/, bool /*bypassed*/)
    {
        project->setModified(true);
    };

    effectRackPanel.onEffectParameterChanged = [this](juce::Uuid /*trackId*/, int /*index*/, int /*paramIndex*/, float /*value*/)
    {
        project->setModified(true);
    };

    // Asset browser callbacks
    assetBrowser.onFileDoubleClicked = [this](const juce::File& file)
    {
        // Import file on double-click
        filesDropped({ file.getFullPathName() }, 0, 0);
    };
}

AudioTrack* MainComponent::getAudioTrackById(const juce::Uuid& id)
{
    for (int i = 0; i < mixer->getNumTracks(); ++i)
    {
        if (auto* audioTrack = mixer->getTrack(i))
        {
            if (audioTrack->getId() == id)
                return audioTrack;
        }
    }
    return nullptr;
}

bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    // Space = Play/Pause toggle
    if (key == juce::KeyPress::spaceKey)
    {
        transport.togglePlayPause();
        return true;
    }

    // Enter/Return = Stop
    if (key == juce::KeyPress::returnKey)
    {
        transport.stop();
        return true;
    }

    // Home = Go to start
    if (key == juce::KeyPress::homeKey)
    {
        transport.setPosition(0.0);
        return true;
    }

    return false;
}

void MainComponent::updateTitle()
{
    juce::String title = "DAW Custom - " + project->getName();

    if (project->isModified())
        title += " *";

    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
        window->setName(title);
}
