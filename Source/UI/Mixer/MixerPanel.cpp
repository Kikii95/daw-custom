#include "MixerPanel.h"
#include "UI/Theme/AppTheme.h"

MixerPanel::MixerPanel()
{
    // Viewport for channel strips
    viewport.setViewedComponent(&stripContainer, false);
    viewport.setScrollBarsShown(false, true);
    addAndMakeVisible(viewport);

    // Master volume
    masterVolume.setSliderStyle(juce::Slider::LinearVertical);
    masterVolume.setRange(0.0, 2.0, 0.01);
    masterVolume.setValue(1.0);
    masterVolume.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    masterVolume.setColour(juce::Slider::thumbColourId, Theme::colour(Theme::accentOrange));
    masterVolume.setTooltip("Master Volume: 0% to 200%");
    masterVolume.onValueChange = [this]()
    {
        if (onMasterVolumeChanged)
            onMasterVolumeChanged(static_cast<float>(masterVolume.getValue()));
    };
    addAndMakeVisible(masterVolume);

    // Master label
    masterLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    masterLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    masterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterLabel);

    // Master meter
    addAndMakeVisible(masterMeter);

    startTimerHz(30);
}

MixerPanel::~MixerPanel()
{
    stopTimer();
}

void MixerPanel::paint(juce::Graphics& g)
{
    g.fillAll(Theme::colour(Theme::bgPanel));

    // Master section background
    auto masterArea = getLocalBounds().removeFromRight(masterWidth);
    g.setColour(Theme::colour(Theme::bgSlot));
    g.fillRect(masterArea);

    // Divider
    g.setColour(Theme::colour(Theme::border));
    g.drawVerticalLine(getWidth() - masterWidth, 0.0f, static_cast<float>(getHeight()));

    // Empty state
    if (channelStrips.empty())
    {
        g.setColour(Theme::Colours::textMuted());
        g.setFont(14.0f);
        auto textArea = getLocalBounds().withTrimmedRight(masterWidth);
        g.drawText("No tracks yet \xe2\x80\x94 Import audio to get started",
                   textArea, juce::Justification::centred);
    }
}

void MixerPanel::resized()
{
    auto bounds = getLocalBounds();

    // Master section on right
    auto masterArea = bounds.removeFromRight(masterWidth).reduced(4);

    masterLabel.setBounds(masterArea.removeFromTop(24));
    masterArea.removeFromTop(4);

    auto meterFaderArea = masterArea;
    masterMeter.setBounds(meterFaderArea.removeFromRight(24));
    meterFaderArea.removeFromRight(4);
    masterVolume.setBounds(meterFaderArea);

    // Channel strips in viewport
    viewport.setBounds(bounds);

    // Size strip container
    int numStrips = static_cast<int>(channelStrips.size());
    int containerWidth = juce::jmax(bounds.getWidth(), numStrips * stripWidth);
    stripContainer.setSize(containerWidth, bounds.getHeight());

    // Layout strips
    int x = 0;
    for (auto& strip : channelStrips)
    {
        strip->setBounds(x, 0, stripWidth, stripContainer.getHeight());
        x += stripWidth;
    }
}

void MixerPanel::setProject(Project* proj)
{
    project = proj;
    refreshChannels();
}

void MixerPanel::setAudioMixer(AudioMixer* mixer)
{
    audioMixer = mixer;
}

void MixerPanel::refreshChannels()
{
    channelStrips.clear();

    if (project == nullptr)
        return;

    for (const auto& track : project->getTracks())
    {
        auto strip = std::make_unique<ChannelStrip>();
        strip->setTrackData(track);

        // Connect callbacks
        strip->onVolumeChanged = [this](juce::Uuid id, float vol)
        {
            if (onTrackVolumeChanged)
                onTrackVolumeChanged(id, vol);
        };

        strip->onPanChanged = [this](juce::Uuid id, float pan)
        {
            if (onTrackPanChanged)
                onTrackPanChanged(id, pan);
        };

        strip->onMuteChanged = [this](juce::Uuid id, bool muted)
        {
            if (onTrackMuteChanged)
                onTrackMuteChanged(id, muted);
        };

        strip->onSoloChanged = [this](juce::Uuid id, bool solo)
        {
            if (onTrackSoloChanged)
                onTrackSoloChanged(id, solo);
        };

        strip->onSelected = [this](juce::Uuid id)
        {
            selectTrack(id);
        };

        stripContainer.addAndMakeVisible(*strip);
        channelStrips.push_back(std::move(strip));
    }

    resized();
}

void MixerPanel::timerCallback()
{
    if (audioMixer == nullptr)
        return;

    // Update master meter
    masterMeter.setLevels(audioMixer->getLeftPeak(), audioMixer->getRightPeak());

    // Update individual track meters (simplified - would need per-track levels)
    // For now, just showing master levels scaled per track
    // Real implementation would get levels from each AudioTrack
}

void MixerPanel::selectTrack(const juce::Uuid& trackId)
{
    selectedTrackId = trackId;

    // Update visual state
    for (auto& strip : channelStrips)
        strip->setSelected(strip->getTrackId() == trackId);

    // Notify callback
    if (onTrackSelected)
        onTrackSelected(trackId);
}
