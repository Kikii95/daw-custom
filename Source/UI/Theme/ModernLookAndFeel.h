#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AppTheme.h"

/**
 * Custom LookAndFeel for DAW Custom.
 * Modern dark theme with glowing accents and smooth interactions.
 */
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();
    ~ModernLookAndFeel() override = default;

    //=========================================================================
    // BUTTONS
    //=========================================================================
    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g,
                        juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    //=========================================================================
    // SLIDERS
    //=========================================================================
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    //=========================================================================
    // COMBOBOX
    //=========================================================================
    void drawComboBox(juce::Graphics& g,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics& g,
                           const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon,
                           const juce::Colour* textColourToUse) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    //=========================================================================
    // LABELS
    //=========================================================================
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    //=========================================================================
    // SCROLLBARS
    //=========================================================================
    void drawScrollbar(juce::Graphics& g,
                       juce::ScrollBar& scrollbar,
                       int x, int y, int width, int height,
                       bool isScrollbarVertical,
                       int thumbStartPosition, int thumbSize,
                       bool isMouseOver, bool isMouseDown) override;

    int getDefaultScrollbarWidth() override { return 10; }

    //=========================================================================
    // TEXT EDITOR
    //=========================================================================
    void fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                  juce::TextEditor& textEditor) override;

    void drawTextEditorOutline(juce::Graphics& g, int width, int height,
                               juce::TextEditor& textEditor) override;

    //=========================================================================
    // TOOLTIP
    //=========================================================================
    void drawTooltip(juce::Graphics& g, const juce::String& text,
                     int width, int height) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernLookAndFeel)
};
