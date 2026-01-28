#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ChannelStrip.h"
#include "MeterComponent.h"
#include "Audio/AudioMixer.h"
#include "Model/Project.h"
#include <vector>
#include <memory>

class MixerPanel : public juce::Component,
                   public juce::Timer
{
public:
    MixerPanel();
    ~MixerPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Project binding
    void setProject(Project* proj);

    // Audio mixer binding (for metering)
    void setAudioMixer(AudioMixer* mixer);

    // Refresh channels from project
    void refreshChannels();

    // Track selection
    void selectTrack(const juce::Uuid& trackId);
    juce::Uuid getSelectedTrackId() const { return selectedTrackId; }

    // Timer for meter updates
    void timerCallback() override;

    // Callbacks for track changes
    std::function<void(juce::Uuid, float)> onTrackVolumeChanged;
    std::function<void(juce::Uuid, float)> onTrackPanChanged;
    std::function<void(juce::Uuid, bool)> onTrackMuteChanged;
    std::function<void(juce::Uuid, bool)> onTrackSoloChanged;
    std::function<void(float)> onMasterVolumeChanged;
    std::function<void(juce::Uuid)> onTrackSelected;

private:
    std::vector<std::unique_ptr<ChannelStrip>> channelStrips;

    // Master section
    juce::Slider masterVolume;
    juce::Label masterLabel { {}, "MASTER" };
    MeterComponent masterMeter;

    Project* project = nullptr;
    AudioMixer* audioMixer = nullptr;
    juce::Uuid selectedTrackId;

    juce::Viewport viewport;
    juce::Component stripContainer;

    static constexpr int stripWidth = 80;
    static constexpr int masterWidth = 100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerPanel)
};
