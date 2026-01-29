#include "DrawingHelpers.h"

namespace DrawingHelpers
{

//=============================================================================
// SHADOW RENDERING
//=============================================================================

void drawShadow(juce::Graphics& g,
                juce::Rectangle<float> bounds,
                const Theme::Shadows::Shadow& shadow,
                float cornerRadius)
{
    if (shadow.alpha <= 0.0f || shadow.blur <= 0.0f)
        return;

    const int numLayers = static_cast<int>(shadow.blur / 2.0f);
    if (numLayers <= 0)
        return;

    const juce::Colour shadowColour = juce::Colour(shadow.colour);

    for (int i = numLayers; i > 0; --i)
    {
        float expansion = static_cast<float>(i) * 1.5f + shadow.spread;
        float layerAlpha = shadow.alpha * (1.0f - static_cast<float>(i) / static_cast<float>(numLayers + 1));

        auto shadowBounds = bounds.expanded(expansion)
                                  .translated(shadow.offsetX, shadow.offsetY);

        g.setColour(shadowColour.withAlpha(layerAlpha * 0.3f));
        g.fillRoundedRectangle(shadowBounds, cornerRadius + expansion * 0.5f);
    }
}

void drawInnerShadow(juce::Graphics& g,
                     juce::Rectangle<float> bounds,
                     float depth,
                     juce::Colour colour,
                     float cornerRadius)
{
    if (depth <= 0.0f)
        return;

    const int numLayers = juce::jmax(3, static_cast<int>(depth));

    for (int i = 0; i < numLayers; ++i)
    {
        float layerDepth = depth * (1.0f - static_cast<float>(i) / static_cast<float>(numLayers));
        float layerAlpha = 0.15f * (1.0f - static_cast<float>(i) / static_cast<float>(numLayers));

        auto insetBounds = bounds.reduced(static_cast<float>(i) * 0.5f);

        g.setColour(colour.withAlpha(layerAlpha));
        g.drawRoundedRectangle(insetBounds, cornerRadius, 1.0f);
    }

    // Top edge darker (light from above)
    juce::ColourGradient topGradient(
        colour.withAlpha(depth * 0.05f), bounds.getX(), bounds.getY(),
        colour.withAlpha(0.0f), bounds.getX(), bounds.getY() + depth,
        false);
    g.setGradientFill(topGradient);
    g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), depth);
}

//=============================================================================
// GLOW EFFECTS
//=============================================================================

void drawGlow(juce::Graphics& g,
              juce::Rectangle<float> bounds,
              juce::Colour colour,
              float radius,
              float intensity)
{
    if (intensity <= 0.0f || radius <= 0.0f)
        return;

    const int numLayers = juce::jmax(3, static_cast<int>(radius / 2.0f));

    for (int i = numLayers; i > 0; --i)
    {
        float expansion = radius * static_cast<float>(i) / static_cast<float>(numLayers);
        float layerAlpha = intensity * (1.0f - static_cast<float>(i - 1) / static_cast<float>(numLayers));

        auto glowBounds = bounds.expanded(expansion);

        g.setColour(colour.withAlpha(layerAlpha * 0.3f));
        g.drawRoundedRectangle(glowBounds, Theme::cornerRadius + expansion * 0.3f, 2.0f);
    }
}

void drawPathGlow(juce::Graphics& g,
                  const juce::Path& path,
                  juce::Colour colour,
                  float radius,
                  float intensity)
{
    if (intensity <= 0.0f || radius <= 0.0f)
        return;

    const int numLayers = juce::jmax(3, static_cast<int>(radius / 3.0f));

    for (int i = numLayers; i > 0; --i)
    {
        float strokeWidth = radius * static_cast<float>(i) / static_cast<float>(numLayers);
        float layerAlpha = intensity * (1.0f - static_cast<float>(i - 1) / static_cast<float>(numLayers));

        g.setColour(colour.withAlpha(layerAlpha * 0.25f));
        g.strokePath(path, juce::PathStrokeType(strokeWidth,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

//=============================================================================
// GRADIENT HELPERS
//=============================================================================

void drawGradientPanel(juce::Graphics& g,
                       juce::Rectangle<float> bounds,
                       const juce::ColourGradient& gradient,
                       float cornerRadius,
                       juce::Colour borderColour)
{
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerRadius);

    if (!borderColour.isTransparent())
    {
        g.setColour(borderColour);
        g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
    }
}

void drawGradientRect(juce::Graphics& g,
                      juce::Rectangle<float> bounds,
                      juce::Colour topColour,
                      juce::Colour bottomColour)
{
    juce::ColourGradient gradient(
        topColour, bounds.getX(), bounds.getY(),
        bottomColour, bounds.getX(), bounds.getBottom(),
        false);

    g.setGradientFill(gradient);
    g.fillRect(bounds);
}

//=============================================================================
// BEVELED BUTTON
//=============================================================================

void drawBeveledButton(juce::Graphics& g,
                       juce::Rectangle<float> bounds,
                       juce::Colour baseColour,
                       bool isPressed,
                       bool isHovered,
                       float cornerRadius)
{
    // Apply state modifiers
    juce::Colour colour = baseColour;
    if (isPressed)
        colour = baseColour.darker(0.1f);
    else if (isHovered)
        colour = baseColour.brighter(0.08f);

    // Drop shadow (only when not pressed)
    if (!isPressed)
    {
        drawShadow(g, bounds, Theme::Shadows::sm, cornerRadius);
    }

    // Gradient fill
    auto gradient = isPressed
        ? Theme::Gradients::buttonPressed(bounds, colour)
        : Theme::Gradients::buttonNormal(bounds, colour);

    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // Top highlight (beveled effect) - only when not pressed
    if (!isPressed)
    {
        drawTopHighlight(g, bounds, cornerRadius, 0.12f);
    }

    // Border
    g.setColour(Theme::colour(Theme::border));
    g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

    // Hover glow
    if (isHovered && !isPressed)
    {
        drawGlow(g, bounds, Theme::Colours::accent(),
                 Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle);
    }
}

//=============================================================================
// UTILITY
//=============================================================================

void drawTopHighlight(juce::Graphics& g,
                      juce::Rectangle<float> bounds,
                      float cornerRadius,
                      float alpha)
{
    // Create clipping path for rounded corners
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);

    juce::Graphics::ScopedSaveState saveState(g);
    g.reduceClipRegion(clipPath);

    // Draw highlight line at top
    g.setColour(juce::Colours::white.withAlpha(alpha));
    g.drawHorizontalLine(static_cast<int>(bounds.getY() + 1),
                         bounds.getX() + cornerRadius,
                         bounds.getRight() - cornerRadius);
}

void drawBottomShadow(juce::Graphics& g,
                      juce::Rectangle<float> bounds,
                      float cornerRadius,
                      float alpha)
{
    // Create clipping path for rounded corners
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);

    juce::Graphics::ScopedSaveState saveState(g);
    g.reduceClipRegion(clipPath);

    // Draw shadow line at bottom
    g.setColour(juce::Colours::black.withAlpha(alpha));
    g.drawHorizontalLine(static_cast<int>(bounds.getBottom() - 1),
                         bounds.getX() + cornerRadius,
                         bounds.getRight() - cornerRadius);
}

} // namespace DrawingHelpers
