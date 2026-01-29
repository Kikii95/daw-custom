#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/Theme/AppTheme.h"
#include <vector>

/**
 * A resizable split panel container.
 * Supports horizontal and vertical splits with draggable dividers.
 */
class FlexPanel : public juce::Component
{
public:
    enum class Orientation { Horizontal, Vertical };

    explicit FlexPanel(Orientation orientation = Orientation::Vertical);
    ~FlexPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    // Panel management
    void addPanel(juce::Component* component, float initialSize = 1.0f);
    void removePanel(juce::Component* component);
    void clearPanels();

    // Layout configuration
    void setOrientation(Orientation newOrientation);
    Orientation getOrientation() const { return orientation; }

    void setDividerSize(int size) { dividerSize = size; resized(); }
    int getDividerSize() const { return dividerSize; }

    void setMinPanelSize(int size) { minPanelSize = size; }
    int getMinPanelSize() const { return minPanelSize; }

    // Get/set panel sizes (proportional, 0.0-1.0)
    std::vector<float> getPanelSizes() const { return panelSizes; }
    void setPanelSizes(const std::vector<float>& sizes);

private:
    struct PanelInfo
    {
        juce::Component* component = nullptr;
        float size = 1.0f;  // Proportional size
    };

    std::vector<PanelInfo> panels;
    std::vector<float> panelSizes;
    Orientation orientation;

    int dividerSize = 6;
    int minPanelSize = 100;

    // Drag state
    int draggingDivider = -1;
    int dragStartPos = 0;
    float dragStartSize1 = 0.0f;
    float dragStartSize2 = 0.0f;

    // Helpers
    int getDividerIndex(int x, int y) const;
    juce::Rectangle<int> getDividerBounds(int index) const;
    void normalizeSizes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexPanel)
};
