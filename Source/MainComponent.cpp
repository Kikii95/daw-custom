#include "MainComponent.h"

MainComponent::MainComponent()
{
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

    // Connect callbacks
    connectCallbacks();

    // Create default track
    addTrack();

    // Set default size
    setSize(1400, 900);

    updateTitle();
}

MainComponent::~MainComponent()
{
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

    // Transport bar (part of main layout, but we position timeline/mixer manually)
    auto transportHeight = 60;
    auto transportArea = bounds.removeFromTop(transportHeight);
    mainLayout.getTransportBar().setBounds(transportArea);

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

        // Create clip
        Clip clip;
        clip.id = juce::Uuid();
        clip.name = file.getFileNameWithoutExtension();
        clip.sourceFile = file;
        clip.startTime = 0.0;
        clip.duration = result.lengthInSeconds;
        clip.sourceSampleRate = result.sampleRate;

        track->addClip(clip);

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
    // TODO: Implement project loading
    DBG("Open Project - Not implemented yet");
}

void MainComponent::saveProject()
{
    if (!project->hasBeenSaved())
    {
        saveProjectAs();
        return;
    }

    // TODO: Implement project saving
    DBG("Save Project - Not implemented yet");
}

void MainComponent::saveProjectAs()
{
    // TODO: Implement save as
    DBG("Save Project As - Not implemented yet");
}

void MainComponent::importAudioFile()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Import Audio File",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        fileLoader.getSupportedExtensions().joinIntoString(";")
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
    mixer->addTrack();

    timelinePanel.refreshTracks();
    mixerPanel.refreshChannels();

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
}

void MainComponent::updateTitle()
{
    juce::String title = "DAW Custom - " + project->getName();

    if (project->isModified())
        title += " *";

    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
        window->setName(title);
}
