#include "ModalOverlay.h"
#include "UI/Theme/DrawingHelpers.h"

ModalOverlay::ModalOverlay()
{
    setInterceptsMouseClicks(true, true);
}

ModalOverlay::~ModalOverlay()
{
    stopTimer();
}

void ModalOverlay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Semi-transparent backdrop
    g.setColour(juce::Colours::black.withAlpha(opacity * 0.75f));
    g.fillAll();

    // Radial vignette (simulated depth/focus effect)
    juce::ColourGradient vignette(
        juce::Colours::transparentBlack,
        bounds.getCentreX(), bounds.getCentreY(),
        juce::Colours::black.withAlpha(opacity * 0.3f),
        0, 0,
        true  // radial
    );
    g.setGradientFill(vignette);
    g.fillAll();

    // Draw content with scale transform if animating
    if (contentComponent != nullptr && contentComponent->isVisible())
    {
        auto contentBounds = contentComponent->getBounds().toFloat();

        // Large shadow behind content
        DrawingHelpers::drawShadow(g, contentBounds, Theme::Shadows::lg, Theme::cornerRadius);

        // Accent glow around content
        if (opacity > 0.5f)
        {
            DrawingHelpers::drawGlow(g, contentBounds, Theme::Colours::accent(),
                                      Theme::Glow::radiusLarge, Theme::Glow::intensitySubtle * opacity);
        }
    }
}

void ModalOverlay::resized()
{
    if (contentComponent != nullptr)
    {
        // Center content with scale applied
        auto contentSize = contentComponent->getLocalBounds();
        int scaledWidth = static_cast<int>(contentSize.getWidth() * scale);
        int scaledHeight = static_cast<int>(contentSize.getHeight() * scale);

        contentComponent->setBounds(
            (getWidth() - scaledWidth) / 2,
            (getHeight() - scaledHeight) / 2,
            scaledWidth,
            scaledHeight
        );
    }
}

void ModalOverlay::show(juce::Component* content, bool animate)
{
    contentComponent = content;
    isHiding = false;

    if (content != nullptr)
    {
        addAndMakeVisible(content);
    }

    if (animate)
    {
        opacity = 0.0f;
        scale = 0.9f;
        targetOpacity = 1.0f;
        targetScale = 1.0f;
        isAnimating = true;
        startTimerHz(60);
    }
    else
    {
        opacity = 1.0f;
        scale = 1.0f;
        isAnimating = false;
    }

    setVisible(true);
    resized();
    repaint();
}

void ModalOverlay::hide(bool animate)
{
    isHiding = true;

    if (animate)
    {
        targetOpacity = 0.0f;
        targetScale = 0.9f;
        isAnimating = true;
        startTimerHz(60);
    }
    else
    {
        opacity = 0.0f;
        scale = 0.9f;
        isAnimating = false;
        setVisible(false);

        if (contentComponent != nullptr)
        {
            removeChildComponent(contentComponent);
            contentComponent = nullptr;
        }
    }
}

void ModalOverlay::mouseDown(const juce::MouseEvent& e)
{
    // Check if click is on backdrop (not on content)
    if (dismissOnBackdropClick && contentComponent != nullptr)
    {
        if (!contentComponent->getBounds().contains(e.getPosition()))
        {
            hide(true);
            if (onDismiss)
                onDismiss();
        }
    }
}

void ModalOverlay::timerCallback()
{
    const float animSpeed = 0.15f;  // Smooth animation speed

    // Animate opacity
    float opacityDiff = targetOpacity - opacity;
    if (std::abs(opacityDiff) > 0.01f)
        opacity += opacityDiff * animSpeed;
    else
        opacity = targetOpacity;

    // Animate scale (easeOutBack-like effect)
    float scaleDiff = targetScale - scale;
    if (std::abs(scaleDiff) > 0.001f)
    {
        // Add slight overshoot for "pop" effect when showing
        float overshoot = isHiding ? 1.0f : 1.1f;
        scale += scaleDiff * animSpeed * overshoot;
    }
    else
    {
        scale = targetScale;
    }

    resized();
    repaint();

    // Check if animation complete
    bool opacityDone = std::abs(opacity - targetOpacity) < 0.01f;
    bool scaleDone = std::abs(scale - targetScale) < 0.001f;

    if (opacityDone && scaleDone)
    {
        stopTimer();
        isAnimating = false;

        if (isHiding)
        {
            setVisible(false);
            if (contentComponent != nullptr)
            {
                removeChildComponent(contentComponent);
                contentComponent = nullptr;
            }
        }
    }
}
