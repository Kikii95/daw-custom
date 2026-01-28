#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ClipComponent.h"
#include "Model/Track.h"
#include "UI/Waveform/WaveformCache.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <memory>

class TrackLane : public juce::Component
{
public:
    TrackLane();
    ~TrackLane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Track data
    void setTrackData(const Track& track);
    const Track& getTrackData() const { return trackData; }

    // Timeline zoom
    void setPixelsPerSecond(double pps);
    double getPixelsPerSecond() const { return pixelsPerSecond; }

    void setVisibleRange(double startTime, double endTime);

    // Clip management
    void addClipComponent(const Clip& clipData, juce::AudioThumbnail* thumbnail);
    void clearClips();

    // Selection
    ClipComponent* getSelectedClip() const { return selectedClip; }

    // Callbacks
    std::function<void(TrackLane*, ClipComponent*)> onClipSelected;

    // Waveform cache access
    void setWaveformCache(WaveformCache* cache) { waveformCache = cache; }
    void setFormatManager(juce::AudioFormatManager* manager) { formatManager = manager; }

private:
    void layoutClips();

    Track trackData;
    std::vector<std::unique_ptr<ClipComponent>> clipComponents;
    ClipComponent* selectedClip = nullptr;

    WaveformCache* waveformCache = nullptr;
    juce::AudioFormatManager* formatManager = nullptr;

    double pixelsPerSecond = 50.0;
    double visibleStart = 0.0;
    double visibleEnd = 30.0;

    // Track header width
    static constexpr int headerWidth = 120;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackLane)
};
