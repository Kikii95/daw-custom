#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/Theme/AppTheme.h"
#include <functional>

/**
 * Modern modal overlay with animated backdrop.
 * Provides semi-transparent background with vignette effect.
 */
class ModalOverlay : public juce::Component,
                     private juce::Timer
{
public:
    ModalOverlay();
    ~ModalOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Show overlay with content (animated)
    void show(juce::Component* content, bool animate = true);

    // Hide overlay (animated)
    void hide(bool animate = true);

    // Callback when overlay dismissed (clicking backdrop)
    std::function<void()> onDismiss;

    // Set whether clicking backdrop dismisses overlay
    void setDismissOnBackdropClick(bool dismiss) { dismissOnBackdropClick = dismiss; }

    // Mouse handling
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;

    juce::Component* contentComponent = nullptr;

    // Animation state
    float opacity = 0.0f;
    float targetOpacity = 0.0f;
    float scale = 0.9f;
    float targetScale = 1.0f;

    bool dismissOnBackdropClick = true;
    bool isAnimating = false;
    bool isHiding = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalOverlay)
};
