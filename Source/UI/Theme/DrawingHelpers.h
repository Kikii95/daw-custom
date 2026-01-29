#pragma once

#include <juce_graphics/juce_graphics.h>
#include "AppTheme.h"

namespace DrawingHelpers
{
    //=========================================================================
    // SHADOW RENDERING
    //=========================================================================

    /** Draw outer drop shadow using layered alpha rects (simulated blur) */
    void drawShadow(juce::Graphics& g,
                    juce::Rectangle<float> bounds,
                    const Theme::Shadows::Shadow& shadow,
                    float cornerRadius);

    /** Draw inner shadow (inset effect) */
    void drawInnerShadow(juce::Graphics& g,
                         juce::Rectangle<float> bounds,
                         float depth,
                         juce::Colour colour,
                         float cornerRadius);

    //=========================================================================
    // GLOW EFFECTS
    //=========================================================================

    /** Draw glow effect around bounds */
    void drawGlow(juce::Graphics& g,
                  juce::Rectangle<float> bounds,
                  juce::Colour colour,
                  float radius,
                  float intensity);

    /** Draw glow along a path */
    void drawPathGlow(juce::Graphics& g,
                      const juce::Path& path,
                      juce::Colour colour,
                      float radius,
                      float intensity);

    //=========================================================================
    // GRADIENT HELPERS
    //=========================================================================

    /** Draw gradient-filled rounded rect with optional border */
    void drawGradientPanel(juce::Graphics& g,
                           juce::Rectangle<float> bounds,
                           const juce::ColourGradient& gradient,
                           float cornerRadius,
                           juce::Colour borderColour = juce::Colour());

    /** Draw panel with top-to-bottom gradient */
    void drawGradientRect(juce::Graphics& g,
                          juce::Rectangle<float> bounds,
                          juce::Colour topColour,
                          juce::Colour bottomColour);

    //=========================================================================
    // BEVELED BUTTON
    //=========================================================================

    /** Draw 3D beveled button with gradient and highlights */
    void drawBeveledButton(juce::Graphics& g,
                           juce::Rectangle<float> bounds,
                           juce::Colour baseColour,
                           bool isPressed,
                           bool isHovered,
                           float cornerRadius);

    //=========================================================================
    // UTILITY
    //=========================================================================

    /** Draw top highlight line (for 3D effect) */
    void drawTopHighlight(juce::Graphics& g,
                          juce::Rectangle<float> bounds,
                          float cornerRadius,
                          float alpha = 0.1f);

    /** Draw bottom shadow line (for 3D effect) */
    void drawBottomShadow(juce::Graphics& g,
                          juce::Rectangle<float> bounds,
                          float cornerRadius,
                          float alpha = 0.15f);
}
