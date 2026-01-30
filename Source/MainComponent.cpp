#include "MainComponent.h"
#include "Audio/AudioClip.h"
#include "Audio/Plugins/PluginManager.h"
#include "UI/Plugins/PluginEditorWindow.h"
#include "Model/ProjectSerializer.h"
#include "Model/Commands/ClipCommands.h"
#include <set>

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

    // Set up oscilloscope with audio callback
    addAndMakeVisible(oscilloscope);
    mixer->setAudioOutputCallback([this](const float* left, const float* right, int numSamples)
    {
        oscilloscope.pushStereoSamples(left, right, numSamples);
    });

    // Connect callbacks
    connectCallbacks();

    // Initialize tempo from project
    double bpm = project->getTempo();
    transport.setTempo(bpm);
    timelinePanel.setTempo(bpm);
    timelinePanel.setTimeSignature(project->getTimeSignatureNumerator(),
                                    project->getTimeSignatureDenominator());
    mainLayout.getTransportBar().setTempo(bpm);

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

    // Oscilloscope above mixer
    auto oscilloscopeHeight = 100;
    oscilloscope.setBounds(bounds.removeFromBottom(oscilloscopeHeight));

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

void MainComponent::fileDragMove(const juce::StringArray& /*files*/, int x, int y)
{
    // Convert to timeline coordinates
    auto localPoint = timelinePanel.getLocalPoint(this, juce::Point<int>(x, y));
    int trackIndex = timelinePanel.getTrackIndexAtY(localPoint.y);

    // Highlight target track
    timelinePanel.setDropTargetTrack(trackIndex);
}

void MainComponent::fileDragExit(const juce::StringArray& /*files*/)
{
    // Clear all drop highlights
    timelinePanel.clearDropTargets();
}

void MainComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
    // Convert window coordinates to timeline panel coordinates
    auto localPoint = timelinePanel.getLocalPoint(this, juce::Point<int>(x, y));

    // Determine target track from Y position
    int targetTrackIndex = timelinePanel.getTrackIndexAtY(localPoint.y);

    // Calculate drop time from X position
    double dropTime = timelinePanel.getTimeAtX(localPoint.x);
    if (dropTime < 0.0)
        dropTime = 0.0;

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

        // Ensure we have at least one track
        if (project->getNumTracks() == 0)
            addTrack();

        // Use target track from drop position, or fallback to first track
        int trackIdx = (targetTrackIndex >= 0 && targetTrackIndex < project->getNumTracks())
                           ? targetTrackIndex
                           : 0;

        auto* track = project->getTrackByIndex(trackIdx);
        if (track == nullptr)
            continue;

        // Create clip metadata
        Clip clip;
        clip.id = juce::Uuid();
        clip.name = file.getFileNameWithoutExtension();
        clip.sourceFile = file;
        clip.startTime = dropTime;  // Use calculated drop time
        clip.duration = result.lengthInSeconds;
        clip.sourceSampleRate = result.sampleRate;
        clip.colour = track->colour;

        // Add to project track (for UI/saving)
        track->addClip(clip);

        // Create AudioClip with actual audio data and add to mixer's AudioTrack
        if (auto* audioTrack = mixer->getTrack(trackIdx))
        {
            auto audioClip = std::make_unique<AudioClip>();
            audioClip->loadFromBuffer(std::move(result.buffer), result.sampleRate, clip);
            audioTrack->addClip(std::move(audioClip));
            audioTrack->setTrackData(*track);
        }

        // Move drop position for next file (stack sequentially)
        dropTime += result.lengthInSeconds;

        // Update duration
        transport.setDuration(project->getTotalDuration());

        // Refresh UI
        timelinePanel.refreshTracks();
        mixerPanel.refreshChannels();

        project->setModified(true);
        updateTitle();
    }

    // Clear drop target highlight
    timelinePanel.clearDropTargets();
}

void MainComponent::importFileToTrack(const juce::File& file, int trackIndex, double startTime)
{
    if (!fileLoader.isFormatSupported(file))
        return;

    auto result = fileLoader.loadFile(file);
    if (!result.isValid())
    {
        DBG("Failed to load: " + result.error);
        return;
    }

    // Ensure we have at least one track
    if (project->getNumTracks() == 0)
        addTrack();

    // Clamp track index
    if (trackIndex < 0)
        trackIndex = 0;
    if (trackIndex >= project->getNumTracks())
        trackIndex = project->getNumTracks() - 1;

    auto* track = project->getTrackByIndex(trackIndex);
    if (!track)
        return;

    // Create clip metadata
    Clip clip;
    clip.id = juce::Uuid();
    clip.name = file.getFileNameWithoutExtension();
    clip.sourceFile = file;
    clip.startTime = startTime;
    clip.duration = result.lengthInSeconds;
    clip.sourceSampleRate = result.sampleRate;
    clip.colour = track->colour;

    // Add to project track
    track->addClip(clip);

    // Create AudioClip and add to mixer's AudioTrack
    if (auto* audioTrack = mixer->getTrack(trackIndex))
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

    // Sync tempo from project to transport and UI
    double bpm = project->getTempo();
    transport.setTempo(bpm);
    timelinePanel.setTempo(bpm);
    timelinePanel.setTimeSignature(project->getTimeSignatureNumerator(),
                                    project->getTimeSignatureDenominator());
    mainLayout.getTransportBar().setTempo(bpm);

    // Create 8 default tracks with distinct colors
    constexpr int numDefaultTracks = 8;
    for (int i = 0; i < numDefaultTracks; ++i)
    {
        auto& track = project->addTrack("Track " + juce::String(i + 1));
        track.colour = Theme::TrackColours::getColour(i);

        auto* audioTrack = mixer->addTrack();
        if (audioTrack != nullptr)
            audioTrack->setTrackData(track);
    }

    timelinePanel.refreshTracks();
    mixerPanel.refreshChannels();
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

                // Sync tempo from project to transport and UI
                double bpm = project->getTempo();
                transport.setTempo(bpm);
                timelinePanel.setTempo(bpm);
                timelinePanel.setTimeSignature(project->getTimeSignatureNumerator(),
                                                project->getTimeSignatureDenominator());
                mainLayout.getTransportBar().setTempo(bpm);

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

    mixerPanel.onTrackReordered = [this](int fromIndex, int toIndex)
    {
        // Reorder in project
        project->moveTrack(fromIndex, toIndex);

        // Reorder in audio mixer
        mixer->moveTrack(fromIndex, toIndex);

        // Refresh UI
        timelinePanel.refreshTracks();
        mixerPanel.refreshChannels();

        project->setModified(true);
        updateTitle();
    };

    // Timeline track selection - sync with mixer and effect rack
    timelinePanel.onTrackSelected = [this](juce::Uuid trackId)
    {
        mixerPanel.selectTrack(trackId);
        if (auto* audioTrack = getAudioTrackById(trackId))
            effectRackPanel.setTrack(audioTrack);
    };

    // Timeline track rename - sync to project, mixer, and audio engine
    timelinePanel.onTrackRenamed = [this](juce::Uuid trackId, const juce::String& newName)
    {
        // Update project model
        if (auto* track = project->getTrack(trackId))
        {
            track->name = newName;
            project->setModified(true);
            updateTitle();
        }

        // Update mixer panel
        mixerPanel.updateTrackName(trackId, newName);

        // Update audio engine track
        if (auto* audioTrack = getAudioTrackById(trackId))
            audioTrack->setName(newName);
    };

    // Timeline clip moved between tracks
    timelinePanel.onClipMoved = [this](juce::Uuid clipId, juce::Uuid fromTrackId, int fromIdx, int toIdx)
    {
        // Get tracks
        auto* fromTrack = project->getTrack(fromTrackId);
        auto* toTrack = project->getTrackByIndex(toIdx);

        if (fromTrack == nullptr || toTrack == nullptr)
            return;

        // Find clip in source track
        Clip clipData;
        bool found = false;
        for (const auto& c : fromTrack->clips)
        {
            if (c.id == clipId)
            {
                clipData = c;
                found = true;
                break;
            }
        }

        if (!found)
            return;

        // Update clip color to match new track
        clipData.colour = toTrack->colour;

        // Remove from source, add to destination (Project model)
        fromTrack->removeClip(clipId);
        toTrack->addClip(clipData);

        // Move in audio engine
        auto* srcAudioTrack = mixer->getTrack(fromIdx);
        auto* dstAudioTrack = mixer->getTrack(toIdx);

        if (srcAudioTrack != nullptr && dstAudioTrack != nullptr)
        {
            auto audioClip = srcAudioTrack->removeClip(clipId);
            if (audioClip != nullptr)
            {
                audioClip->setClipData(clipData);  // Update color
                dstAudioTrack->addClip(std::move(audioClip));
            }
        }

        // Refresh UI
        timelinePanel.refreshTracks();

        project->setModified(true);
        updateTitle();
    };

    // Clip time moved (horizontal drag)
    timelinePanel.onClipTimeMoved = [this](juce::Uuid trackId, juce::Uuid clipId, double newStartTime)
    {
        auto* track = project->getTrack(trackId);
        if (track == nullptr)
            return;

        // Update clip in project
        for (auto& clip : track->clips)
        {
            if (clip.id == clipId)
            {
                clip.startTime = newStartTime;
                break;
            }
        }

        // Update audio engine clip
        if (auto* audioTrack = mixer->getTrackById(trackId))
        {
            if (auto* audioClip = audioTrack->getClip(clipId))
            {
                Clip clipData = audioClip->getClipData();
                clipData.startTime = newStartTime;
                audioClip->setClipData(clipData);
            }
        }

        // Update total duration
        transport.setDuration(project->getTotalDuration());

        project->setModified(true);
        updateTitle();
    };

    // Clip move complete (for undo system) - called at end of drag
    timelinePanel.onClipMoveComplete = [this](std::vector<TimelinePanel::ClipPositionChange> changes)
    {
        // Convert to MoveClipCommand format
        std::vector<MoveClipCommand::ClipPosition> oldPositions;
        std::vector<MoveClipCommand::ClipPosition> newPositions;

        for (const auto& change : changes)
        {
            oldPositions.push_back({change.trackId, change.clipId, change.oldStartTime});
            newPositions.push_back({change.trackId, change.clipId, change.newStartTime});
        }

        // Create and execute command (already applied, so we just record it)
        auto cmd = std::make_unique<MoveClipCommand>(project.get(), mixer.get(),
                                                      std::move(oldPositions), std::move(newPositions));
        // Don't call execute() - positions are already updated
        // Just add to undo stack
        undoManager.execute(std::move(cmd));
    };

    // Track delete from context menu
    timelinePanel.onTrackDelete = [this](juce::Uuid trackId)
    {
        // Remove from project and mixer (both use UUID)
        project->removeTrack(trackId);
        mixer->removeTrack(trackId);

        // Update UI
        timelinePanel.refreshTracks();
        mixerPanel.refreshChannels();
        effectRackPanel.setTrack(nullptr);

        // Update duration
        transport.setDuration(project->getTotalDuration());

        project->setModified(true);
        updateTitle();
    };

    // Track colour change from context menu
    timelinePanel.onTrackColourChanged = [this](juce::Uuid trackId, juce::Colour newColour)
    {
        if (auto* track = project->getTrack(trackId))
        {
            track->colour = newColour;

            // Update all clips on this track
            for (auto& clip : track->clips)
                clip.colour = newColour;

            timelinePanel.refreshTracks();
            project->setModified(true);
            updateTitle();
        }
    };

    // Clip delete from context menu (with undo support)
    timelinePanel.onClipDelete = [this](juce::Uuid trackId, juce::Uuid clipId)
    {
        std::vector<std::pair<juce::Uuid, juce::Uuid>> clipsToDelete;
        clipsToDelete.push_back({trackId, clipId});

        auto cmd = std::make_unique<DeleteClipsCommand>(project.get(), mixer.get(), &fileLoader,
                                                        std::move(clipsToDelete));
        undoManager.execute(std::move(cmd));

        // Update UI
        timelinePanel.refreshTracks();
        transport.setDuration(project->getTotalDuration());

        project->setModified(true);
        updateTitle();
    };

    // Clip duplicate from context menu (with undo support)
    timelinePanel.onClipDuplicate = [this](juce::Uuid trackId, juce::Uuid clipId)
    {
        auto* track = project->getTrack(trackId);
        if (track == nullptr)
            return;

        // Find original clip
        const Clip* originalClip = nullptr;
        for (const auto& c : track->clips)
        {
            if (c.id == clipId)
            {
                originalClip = &c;
                break;
            }
        }

        if (originalClip == nullptr)
            return;

        // Create duplicate
        Clip newClip = *originalClip;
        newClip.id = juce::Uuid();
        newClip.name = originalClip->name + " (copy)";
        newClip.startTime = originalClip->getEndTime();  // Place after original

        // Execute via undo system
        auto cmd = std::make_unique<AddClipCommand>(project.get(), mixer.get(), &fileLoader,
                                                     trackId, newClip);
        undoManager.execute(std::move(cmd));

        // Update UI
        timelinePanel.refreshTracks();
        transport.setDuration(project->getTotalDuration());

        project->setModified(true);
        updateTitle();
    };

    // Clip split (Ctrl+B)
    timelinePanel.onClipSplit = [this](juce::Uuid trackId, juce::Uuid clipId, double splitTime)
    {
        auto* track = project->getTrack(trackId);
        if (track == nullptr)
            return;

        // Find original clip
        const Clip* originalClip = nullptr;
        for (const auto& c : track->clips)
        {
            if (c.id == clipId)
            {
                originalClip = &c;
                break;
            }
        }

        if (originalClip == nullptr)
            return;

        // Check if split time is valid (within clip bounds)
        if (splitTime <= originalClip->startTime || splitTime >= originalClip->getEndTime())
            return;

        // Create Clip A: from original start to split point
        Clip clipA = *originalClip;
        clipA.id = juce::Uuid();
        clipA.duration = splitTime - originalClip->startTime;
        // sourceStartOffset stays the same

        // Create Clip B: from split point to original end
        Clip clipB = *originalClip;
        clipB.id = juce::Uuid();
        clipB.startTime = splitTime;
        clipB.duration = originalClip->getEndTime() - splitTime;
        // Adjust sourceStartOffset to account for the portion we skipped
        clipB.sourceStartOffset = originalClip->sourceStartOffset + (splitTime - originalClip->startTime);

        // Find track index for audio engine
        int trackIdx = -1;
        for (int i = 0; i < project->getNumTracks(); ++i)
        {
            if (auto* t = project->getTrackByIndex(i); t && t->id == trackId)
            {
                trackIdx = i;
                break;
            }
        }

        // Remove original clip from project
        track->removeClip(clipId);

        // Add new clips to project
        track->addClip(clipA);
        track->addClip(clipB);

        // Update audio engine
        if (trackIdx >= 0)
        {
            if (auto* audioTrack = mixer->getTrack(trackIdx))
            {
                // Remove original audio clip
                audioTrack->removeClip(clipId);

                // Add new audio clips
                if (clipA.sourceFile.existsAsFile())
                {
                    auto result = fileLoader.loadFile(clipA.sourceFile);
                    if (result.isValid())
                    {
                        auto newAudioClip = std::make_unique<AudioClip>();
                        newAudioClip->loadFromBuffer(std::move(result.buffer), result.sampleRate, clipA);
                        audioTrack->addClip(std::move(newAudioClip));
                    }
                }

                if (clipB.sourceFile.existsAsFile())
                {
                    auto result = fileLoader.loadFile(clipB.sourceFile);
                    if (result.isValid())
                    {
                        auto newAudioClip = std::make_unique<AudioClip>();
                        newAudioClip->loadFromBuffer(std::move(result.buffer), result.sampleRate, clipB);
                        audioTrack->addClip(std::move(newAudioClip));
                    }
                }
            }
        }

        // Update UI
        timelinePanel.refreshTracks();
        transport.setDuration(project->getTotalDuration());

        project->setModified(true);
        updateTitle();
    };

    // Clip trim callback - update model and audio
    timelinePanel.onClipTrimmed = [this](juce::Uuid trackId, juce::Uuid clipId,
                                          double newStart, double newDuration, double newSourceOffset)
    {
        auto* track = project->getTrack(trackId);
        if (track == nullptr)
            return;

        // Update clip in project model
        for (auto& clip : track->clips)
        {
            if (clip.id == clipId)
            {
                clip.startTime = newStart;
                clip.duration = newDuration;
                clip.sourceStartOffset = newSourceOffset;
                break;
            }
        }

        // Find track index for audio engine
        int trackIdx = -1;
        for (int i = 0; i < project->getNumTracks(); ++i)
        {
            if (auto* t = project->getTrackByIndex(i); t && t->id == trackId)
            {
                trackIdx = i;
                break;
            }
        }

        // Update audio clip timing
        if (trackIdx >= 0)
        {
            if (auto* audioTrack = mixer->getTrack(trackIdx))
            {
                audioTrack->updateClipTiming(clipId, newStart, newDuration, newSourceOffset);
            }
        }

        transport.setDuration(project->getTotalDuration());
        project->setModified(true);
        updateTitle();
    };

    // Clip fade changed callback
    timelinePanel.onClipFadeChanged = [this](juce::Uuid trackId, juce::Uuid clipId,
                                              bool isFadeIn, double duration)
    {
        auto* track = project->getTrack(trackId);
        if (track == nullptr)
            return;

        // Update clip in project model
        for (auto& clip : track->clips)
        {
            if (clip.id == clipId)
            {
                if (isFadeIn)
                    clip.fadeInDuration = duration;
                else
                    clip.fadeOutDuration = duration;
                break;
            }
        }

        // Find track index for audio engine
        int trackIdx = -1;
        for (int i = 0; i < project->getNumTracks(); ++i)
        {
            if (auto* t = project->getTrackByIndex(i); t && t->id == trackId)
            {
                trackIdx = i;
                break;
            }
        }

        // Update audio clip fade
        if (trackIdx >= 0)
        {
            if (auto* audioTrack = mixer->getTrack(trackIdx))
            {
                audioTrack->updateClipFade(clipId, isFadeIn, duration);
            }
        }

        project->setModified(true);
        updateTitle();
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
        // Import file to selected track at playhead position
        int targetTrackIndex = 0;
        auto selectedTrackId = mixerPanel.getSelectedTrackId();

        if (!selectedTrackId.isNull())
        {
            for (int i = 0; i < project->getNumTracks(); ++i)
            {
                if (auto* t = project->getTrackByIndex(i); t && t->id == selectedTrackId)
                {
                    targetTrackIndex = i;
                    break;
                }
            }
        }

        double insertTime = transport.getPosition();
        importFileToTrack(file, targetTrackIndex, insertTime);
    };

    // Timeline loop callbacks
    timelinePanel.onLoopRangeChanged = [this](double start, double end)
    {
        transport.setLoopRange(start, end);
        transport.setLoopEnabled(true);
        project->setModified(true);
        updateTitle();
    };

    timelinePanel.onPositionClicked = [this](double time)
    {
        transport.setPosition(time);
    };

    // Internal drag-drop from AssetBrowser to Timeline
    timelinePanel.onFileDropped = [this](const juce::File& file, int trackIndex, double time)
    {
        importFileToTrack(file, trackIndex, time);
    };

    // Tempo synchronization callback
    mainLayout.getTransportBar().onTempoChanged = [this](double bpm)
    {
        project->setTempo(bpm);
        timelinePanel.setTempo(bpm);
        effectRackPanel.setTempo(bpm);  // For delay tempo sync
        project->setModified(true);
        updateTitle();
    };

    // Snap value callback
    mainLayout.getTransportBar().onSnapValueChanged = [this](int snapIndex)
    {
        timelinePanel.setSnapValue(static_cast<SnapValue>(snapIndex));
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

    // Ctrl+S = Save
    if (key == juce::KeyPress('s', juce::ModifierKeys::ctrlModifier, 0))
    {
        saveProject();
        return true;
    }

    // Ctrl+O = Open
    if (key == juce::KeyPress('o', juce::ModifierKeys::ctrlModifier, 0))
    {
        openProject();
        return true;
    }

    // Ctrl+N = New
    if (key == juce::KeyPress('n', juce::ModifierKeys::ctrlModifier, 0))
    {
        newProject();
        return true;
    }

    // Ctrl+T = Add Track
    if (key == juce::KeyPress('t', juce::ModifierKeys::ctrlModifier, 0))
    {
        addTrack();
        return true;
    }

    // Delete or Backspace = Delete selected clips
    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
    {
        auto& selectedClips = timelinePanel.getSelectedClips();
        if (!selectedClips.empty())
        {
            // Make a copy since we'll be modifying during iteration
            auto clipsToDelete = selectedClips;
            for (const auto& [trackId, clipId] : clipsToDelete)
            {
                // Delete from project
                if (auto* track = project->getTrack(trackId))
                {
                    track->clips.erase(
                        std::remove_if(track->clips.begin(), track->clips.end(),
                            [&clipId](const Clip& c) { return c.id == clipId; }),
                        track->clips.end());
                }
                // Delete from audio engine
                if (auto* audioTrack = getAudioTrackById(trackId))
                    audioTrack->removeClip(clipId);
            }
            timelinePanel.clearClipSelection();
            timelinePanel.refreshTracks();
            project->setModified(true);
            updateTitle();
            return true;
        }
        return false;
    }

    // Zoom shortcuts: . = zoom in, , = zoom out
    if (key.getTextCharacter() == '.')
    {
        timelinePanel.zoomIn();
        return true;
    }
    if (key.getTextCharacter() == ',')
    {
        timelinePanel.zoomOut();
        return true;
    }

    // Arrow keys = nudge selected clips
    if (key == juce::KeyPress::leftKey || key == juce::KeyPress::rightKey)
    {
        auto& selectedClips = timelinePanel.getSelectedClips();
        if (!selectedClips.empty())
        {
            // Nudge amount: Shift = fine (0.01s), normal = grid snap interval
            double nudgeAmount = key.getModifiers().isShiftDown() ? 0.01 : timelinePanel.getSnapInterval();
            if (key == juce::KeyPress::leftKey)
                nudgeAmount = -nudgeAmount;

            for (const auto& [trackId, clipId] : selectedClips)
            {
                if (auto* track = project->getTrack(trackId))
                {
                    for (auto& clip : track->clips)
                    {
                        if (clip.id == clipId)
                        {
                            clip.startTime = juce::jmax(0.0, clip.startTime + nudgeAmount);
                            break;
                        }
                    }
                }
                if (auto* audioTrack = getAudioTrackById(trackId))
                {
                    if (auto* audioClip = audioTrack->getClip(clipId))
                    {
                        auto clipData = audioClip->getClipData();
                        clipData.startTime = juce::jmax(0.0, clipData.startTime + nudgeAmount);
                        audioClip->setClipData(clipData);
                    }
                }
            }
            timelinePanel.refreshTracks();
            project->setModified(true);
            updateTitle();
            return true;
        }
    }

    // M = Mute selected track
    if (key.getKeyCode() == 'M' && !key.getModifiers().isCtrlDown())
    {
        auto selectedTrackId = mixerPanel.getSelectedTrackId();
        if (!selectedTrackId.isNull())
        {
            if (auto* track = project->getTrack(selectedTrackId))
            {
                track->muted = !track->muted;
                if (auto* audioTrack = getAudioTrackById(selectedTrackId))
                    audioTrack->setMuted(track->muted);
                mixerPanel.refreshChannels();
            }
        }
        return true;
    }

    // S = Solo selected track (only when not Ctrl+S)
    if (key.getKeyCode() == 'S' && !key.getModifiers().isCtrlDown())
    {
        auto selectedTrackId = mixerPanel.getSelectedTrackId();
        if (!selectedTrackId.isNull())
        {
            if (auto* track = project->getTrack(selectedTrackId))
            {
                track->solo = !track->solo;
                if (auto* audioTrack = getAudioTrackById(selectedTrackId))
                    audioTrack->setSolo(track->solo);
                mixerPanel.refreshChannels();
            }
        }
        return true;
    }

    // L = Toggle loop mode
    if (key.getKeyCode() == 'L' && !key.getModifiers().isCtrlDown())
    {
        transport.setLoopEnabled(!transport.isLoopEnabled());
        return true;
    }

    // G = Toggle snap to grid
    if (key.getKeyCode() == 'G' && !key.getModifiers().isCtrlDown())
    {
        timelinePanel.setSnapEnabled(!timelinePanel.isSnapEnabled());
        return true;
    }

    // Ctrl+C = Copy selected clips
    if (key == juce::KeyPress('c', juce::ModifierKeys::ctrlModifier, 0))
    {
        copySelectedClips();
        return true;
    }

    // Ctrl+V = Paste clips
    if (key == juce::KeyPress('v', juce::ModifierKeys::ctrlModifier, 0))
    {
        pasteClips();
        return true;
    }

    // Ctrl+B = Split selected clips at playhead
    if (key == juce::KeyPress('b', juce::ModifierKeys::ctrlModifier, 0))
    {
        timelinePanel.splitSelectedClipsAtPlayhead();
        return true;
    }

    // Ctrl+Z = Undo
    if (key == juce::KeyPress('z', juce::ModifierKeys::ctrlModifier, 0))
    {
        if (undoManager.canUndo())
        {
            undoManager.undo();
            timelinePanel.refreshTracks();
            transport.setDuration(project->getTotalDuration());
            project->setModified(true);
            updateTitle();
        }
        return true;
    }

    // Ctrl+Y or Ctrl+Shift+Z = Redo
    if (key == juce::KeyPress('y', juce::ModifierKeys::ctrlModifier, 0) ||
        key == juce::KeyPress('z', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        if (undoManager.canRedo())
        {
            undoManager.redo();
            timelinePanel.refreshTracks();
            transport.setDuration(project->getTotalDuration());
            project->setModified(true);
            updateTitle();
        }
        return true;
    }

    // Ctrl+Shift+A = Select all clips globally
    if (key == juce::KeyPress('a', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        timelinePanel.selectAllClips();
        return true;
    }

    // Ctrl+A = Select all clips on selected track
    if (key == juce::KeyPress('a', juce::ModifierKeys::ctrlModifier, 0))
    {
        auto selectedTrackId = mixerPanel.getSelectedTrackId();
        if (!selectedTrackId.isNull())
        {
            timelinePanel.clearClipSelection();
            timelinePanel.selectAllClipsOnTrack(selectedTrackId);
            return true;
        }
        return false;
    }

    // Escape = Clear selection
    if (key == juce::KeyPress::escapeKey)
    {
        timelinePanel.clearClipSelection();
        return true;
    }

    // Ctrl+D = Duplicate selected clips
    if (key == juce::KeyPress('d', juce::ModifierKeys::ctrlModifier, 0))
    {
        duplicateSelectedClips();
        return true;
    }

    // + / = = Zoom in (numpad plus or equals)
    if (key.getKeyCode() == juce::KeyPress::numberPadAdd ||
        key.getTextCharacter() == '=' || key.getTextCharacter() == '+')
    {
        timelinePanel.zoomIn();
        return true;
    }

    // - = Zoom out (numpad minus or minus)
    if (key.getKeyCode() == juce::KeyPress::numberPadSubtract ||
        key.getTextCharacter() == '-')
    {
        timelinePanel.zoomOut();
        return true;
    }

    // Ctrl+0 = Zoom to fit all clips
    if (key == juce::KeyPress('0', juce::ModifierKeys::ctrlModifier, 0) ||
        key.getKeyCode() == juce::KeyPress::numberPad0 && key.getModifiers().isCtrlDown())
    {
        timelinePanel.zoomToFitAll();
        return true;
    }

    // Ctrl+1 = Zoom to selection
    if (key == juce::KeyPress('1', juce::ModifierKeys::ctrlModifier, 0))
    {
        timelinePanel.zoomToSelection();
        return true;
    }

    // Ctrl+G = Group selected clips
    if (key == juce::KeyPress('g', juce::ModifierKeys::ctrlModifier, 0))
    {
        const auto& selected = timelinePanel.getSelectedClips();
        if (selected.size() >= 2)
        {
            project->createGroup(selected);
            project->setModified(true);
            updateTitle();
            timelinePanel.repaint();
        }
        return true;
    }

    // Ctrl+Shift+G = Ungroup selected clips
    if (key == juce::KeyPress('g', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        const auto& selected = timelinePanel.getSelectedClips();
        if (!selected.empty())
        {
            // Find and dissolve groups for selected clips
            std::set<juce::Uuid> groupsToDissolve;
            for (const auto& [trackId, clipId] : selected)
            {
                if (auto* group = project->getGroupForClip(trackId, clipId))
                    groupsToDissolve.insert(group->id);
            }

            for (const auto& groupId : groupsToDissolve)
                project->dissolveGroup(groupId);

            if (!groupsToDissolve.empty())
            {
                project->setModified(true);
                updateTitle();
                timelinePanel.repaint();
            }
        }
        return true;
    }

    // R = Toggle Ripple Edit mode
    if (key.getTextCharacter() == 'r' && !key.getModifiers().isAnyModifierKeyDown())
    {
        auto currentMode = project->getEditMode();
        auto newMode = (currentMode == EditMode::Normal) ? EditMode::Ripple : EditMode::Normal;
        project->setEditMode(newMode);

        // Show notification
        auto message = newMode == EditMode::Ripple
            ? "Ripple Edit: ON"
            : "Ripple Edit: OFF";
        DBG(message);  // TODO: Add visual indicator in transport bar
        return true;
    }

    // Shift+M = Add marker at playhead position
    if (key == juce::KeyPress('m', juce::ModifierKeys::shiftModifier, 0))
    {
        double playheadTime = transport.getPosition();
        project->addMarker(playheadTime);
        timelinePanel.repaint();
        project->setModified(true);
        updateTitle();
        return true;
    }

    // Numpad 1-9 = Jump to marker with that shortcut number
    int numpadKey = -1;
    if (key.getKeyCode() == juce::KeyPress::numberPad1) numpadKey = 1;
    else if (key.getKeyCode() == juce::KeyPress::numberPad2) numpadKey = 2;
    else if (key.getKeyCode() == juce::KeyPress::numberPad3) numpadKey = 3;
    else if (key.getKeyCode() == juce::KeyPress::numberPad4) numpadKey = 4;
    else if (key.getKeyCode() == juce::KeyPress::numberPad5) numpadKey = 5;
    else if (key.getKeyCode() == juce::KeyPress::numberPad6) numpadKey = 6;
    else if (key.getKeyCode() == juce::KeyPress::numberPad7) numpadKey = 7;
    else if (key.getKeyCode() == juce::KeyPress::numberPad8) numpadKey = 8;
    else if (key.getKeyCode() == juce::KeyPress::numberPad9) numpadKey = 9;

    if (numpadKey > 0)
    {
        if (auto* marker = project->getMarkerByShortcut(numpadKey))
        {
            transport.setPosition(marker->time);
            return true;
        }
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

void MainComponent::copySelectedClips()
{
    clipboardClips.clear();

    const auto& selectedClips = timelinePanel.getSelectedClips();
    if (selectedClips.empty())
        return;

    // Copy clip data from selected clips
    for (const auto& [trackId, clipId] : selectedClips)
    {
        if (auto* track = project->getTrack(trackId))
        {
            for (const auto& clip : track->clips)
            {
                if (clip.id == clipId)
                {
                    clipboardClips.push_back(clip);
                    break;
                }
            }
        }
    }
}

void MainComponent::pasteClips()
{
    if (clipboardClips.empty())
        return;

    // Use click position if recent, otherwise use playhead
    double pasteTime;
    if (timelinePanel.hasRecentClick())
        pasteTime = timelinePanel.getLastClickTime();
    else
        pasteTime = transport.getPosition();

    // Find earliest clip time in clipboard for offset calculation
    double earliestTime = std::numeric_limits<double>::max();
    for (const auto& clip : clipboardClips)
        earliestTime = juce::jmin(earliestTime, clip.startTime);

    // Get target track: clicked track > selected track > first track
    juce::Uuid targetTrackId;
    int clickedTrackIdx = timelinePanel.getLastClickTrackIndex();

    if (timelinePanel.hasRecentClick() && clickedTrackIdx >= 0 &&
        clickedTrackIdx < project->getNumTracks())
    {
        targetTrackId = project->getTrackByIndex(clickedTrackIdx)->id;
    }
    else
    {
        auto selectedTrackId = mixerPanel.getSelectedTrackId();
        if (!selectedTrackId.isNull())
            targetTrackId = selectedTrackId;
        else if (project->getNumTracks() > 0)
            targetTrackId = project->getTrackByIndex(0)->id;
        else
            return;
    }

    auto* targetTrack = project->getTrack(targetTrackId);
    if (targetTrack == nullptr)
        return;

    // Find target audio track index
    int targetTrackIdx = -1;
    for (int i = 0; i < project->getNumTracks(); ++i)
    {
        if (auto* t = project->getTrackByIndex(i); t && t->id == targetTrackId)
        {
            targetTrackIdx = i;
            break;
        }
    }

    if (targetTrackIdx < 0)
        return;

    // Paste each clip
    for (const auto& sourceClip : clipboardClips)
    {
        // Create new clip with new ID and adjusted time
        Clip newClip = sourceClip;
        newClip.id = juce::Uuid();
        newClip.startTime = pasteTime + (sourceClip.startTime - earliestTime);
        newClip.name = sourceClip.name + " (paste)";
        newClip.colour = targetTrack->colour;

        // Add to project
        targetTrack->addClip(newClip);

        // Add to audio engine
        if (auto* audioTrack = mixer->getTrack(targetTrackIdx))
        {
            if (newClip.sourceFile.existsAsFile())
            {
                auto result = fileLoader.loadFile(newClip.sourceFile);
                if (result.isValid())
                {
                    auto audioClip = std::make_unique<AudioClip>();
                    audioClip->loadFromBuffer(std::move(result.buffer), result.sampleRate, newClip);
                    audioTrack->addClip(std::move(audioClip));
                }
            }
        }
    }

    // Update UI
    timelinePanel.refreshTracks();
    transport.setDuration(project->getTotalDuration());

    project->setModified(true);
    updateTitle();
}

void MainComponent::duplicateSelectedClips()
{
    const auto& selected = timelinePanel.getSelectedClips();
    if (selected.empty())
        return;

    // Find max end time and earliest start for positioning
    double maxEndTime = 0.0;
    double earliestStart = std::numeric_limits<double>::max();

    for (const auto& [trackId, clipId] : selected)
    {
        if (auto* track = project->getTrack(trackId))
        {
            for (const auto& clip : track->clips)
            {
                if (clip.id == clipId)
                {
                    maxEndTime = juce::jmax(maxEndTime, clip.getEndTime());
                    earliestStart = juce::jmin(earliestStart, clip.startTime);
                }
            }
        }
    }

    // Clear selection before adding new clips
    timelinePanel.clearClipSelection();

    // Duplicate each selected clip
    std::vector<std::pair<juce::Uuid, juce::Uuid>> newClips;

    for (const auto& [trackId, clipId] : selected)
    {
        if (auto* track = project->getTrack(trackId))
        {
            for (const auto& original : track->clips)
            {
                if (original.id == clipId)
                {
                    // Create duplicate clip
                    Clip newClip = original;
                    newClip.id = juce::Uuid();
                    newClip.startTime = maxEndTime + (original.startTime - earliestStart);
                    newClip.name = original.name + " (copy)";

                    // Execute via undo system
                    auto cmd = std::make_unique<AddClipCommand>(project.get(), mixer.get(), &fileLoader,
                                                                 trackId, newClip);
                    undoManager.execute(std::move(cmd));

                    newClips.push_back({trackId, newClip.id});
                    break;
                }
            }
        }
    }

    // Update UI
    timelinePanel.refreshTracks();
    transport.setDuration(project->getTotalDuration());

    // Select the new clips
    for (const auto& [trackId, clipId] : newClips)
        timelinePanel.addToSelection(trackId, clipId);

    project->setModified(true);
    updateTitle();
}
