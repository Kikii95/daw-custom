#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ClipComponent.h"
#include "Model/Track.h"
#include "UI/Waveform/WaveformCache.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <memory>

class TrackLane : public juce::Component,
                  private juce::TextEditor::Listener
{
public:
    TrackLane();
    ~TrackLane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Mouse handling
    void mouseDoubleClick(const juce::MouseEvent& e) override;

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
    std::function<void(TrackLane*, const juce::String&)> onTrackRenamed;

    // Waveform cache access
    void setWaveformCache(WaveformCache* cache) { waveformCache = cache; }
    void setFormatManager(juce::AudioFormatManager* manager) { formatManager = manager; }

    // Header width accessor for ruler alignment
    static constexpr int getHeaderWidth() { return headerWidth; }

private:
    void layoutClips();
    void showNameEditor();
    void hideNameEditor();
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;

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

    // Name editing
    std::unique_ptr<juce::TextEditor> nameEditor;
    bool isEditingName = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackLane)
};
