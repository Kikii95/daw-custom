#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Model/Clip.h"
#include "UI/Waveform/WaveformDisplay.h"

class ClipComponent : public juce::Component
{
public:
    ClipComponent();
    ~ClipComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Clip data
    void setClipData(const Clip& clip);
    const Clip& getClipData() const { return clipData; }

    // Waveform
    void setThumbnail(juce::AudioThumbnail* thumb);

    // Selection
    void setSelected(bool selected);
    bool isSelected() const { return selected; }

    // Mouse handling
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // Callback for drag
    std::function<void(ClipComponent*, double)> onDrag;
    std::function<void(ClipComponent*)> onSelect;

private:
    Clip clipData;
    WaveformDisplay waveformDisplay;
    bool selected = false;
    bool dragging = false;

    juce::Point<int> dragStart;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};
