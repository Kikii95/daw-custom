#include "ClipComponent.h"
#include "UI/Theme/AppTheme.h"
#include "UI/Theme/DrawingHelpers.h"

ClipComponent::ClipComponent()
{
    addAndMakeVisible(waveformDisplay);
}

ClipComponent::~ClipComponent()
{
}

void ClipComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Drop shadow
    DrawingHelpers::drawShadow(g, bounds,
                                selected ? Theme::Shadows::md : Theme::Shadows::sm,
                                Theme::cornerRadiusSm);

    auto innerBounds = bounds.reduced(1);

    // Background gradient
    auto bgGradient = juce::ColourGradient(
        clipData.colour.brighter(0.15f), innerBounds.getX(), innerBounds.getY(),
        clipData.colour.darker(0.2f), innerBounds.getX(), innerBounds.getBottom(),
        false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(innerBounds, Theme::cornerRadiusSm);

    // Darker header band (top 18px)
    auto headerBounds = innerBounds.removeFromTop(18);
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(headerBounds.withTrimmedBottom(-4), Theme::cornerRadiusSm);

    // Top highlight line (beveled effect)
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawHorizontalLine(static_cast<int>(bounds.getY() + 2),
                         bounds.getX() + 4, bounds.getRight() - 4);

    // Clip name with subtle text shadow
    auto textBounds = bounds.reduced(4, 2).removeFromTop(16);
    g.setFont(juce::Font(11.0f, juce::Font::bold));

    // Text shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawText(clipData.name,
               textBounds.translated(1, 1),
               juce::Justification::centredLeft,
               true);

    // Text
    g.setColour(Theme::Colours::text());
    g.drawText(clipData.name,
               textBounds,
               juce::Justification::centredLeft,
               true);

    // Selection or hover glow
    if (selected)
    {
        DrawingHelpers::drawGlow(g, bounds.reduced(1), Theme::Colours::accent(),
                                  Theme::Glow::radiusMedium, Theme::Glow::intensityNormal);

        g.setColour(Theme::Colours::accent());
        g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadiusSm, 2.0f);
    }
    else if (hovered)
    {
        // Subtle hover glow
        DrawingHelpers::drawGlow(g, bounds.reduced(1), clipData.colour.brighter(0.3f),
                                  Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle);

        g.setColour(clipData.colour.brighter(0.2f));
        g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadiusSm, 1.5f);
    }
    else
    {
        // Subtle border
        g.setColour(clipData.colour.darker(0.4f));
        g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadiusSm, 1.0f);
    }

    // Trim handles visual feedback
    if (hoveredEdge != EdgeHover::None || trimming)
    {
        auto handleColour = Theme::Colours::accent().withAlpha(0.7f);
        auto handleWidth = static_cast<float>(edgeHitZone);

        if (hoveredEdge == EdgeHover::Left || trimEdge == EdgeHover::Left)
        {
            auto leftHandle = bounds.withWidth(handleWidth);
            g.setColour(handleColour);
            g.fillRoundedRectangle(leftHandle, Theme::cornerRadiusSm);
        }

        if (hoveredEdge == EdgeHover::Right || trimEdge == EdgeHover::Right)
        {
            auto rightHandle = bounds.withTrimmedLeft(bounds.getWidth() - handleWidth);
            g.setColour(handleColour);
            g.fillRoundedRectangle(rightHandle, Theme::cornerRadiusSm);
        }
    }

    // Draw fade overlays
    paintFadeOverlays(g, bounds);
}

void ClipComponent::paintFadeOverlays(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    const float fadeInWidth = static_cast<float>(timeToPixels(clipData.fadeInDuration));
    const float fadeOutWidth = static_cast<float>(timeToPixels(clipData.fadeOutDuration));

    // Only draw if there are fades
    if (fadeInWidth < 2.0f && fadeOutWidth < 2.0f && hoveredFade == FadeHover::None)
        return;

    const float maxHeight = bounds.getHeight() * 0.5f;  // Fade triangles go halfway down

    // Fade In triangle (top-left corner)
    if (fadeInWidth >= 2.0f || hoveredFade == FadeHover::FadeIn || draggingFade)
    {
        float actualWidth = juce::jmax(fadeInWidth, 4.0f);  // Minimum visible width

        juce::Path fadeInPath;
        fadeInPath.startNewSubPath(bounds.getX(), bounds.getY());
        fadeInPath.lineTo(bounds.getX() + actualWidth, bounds.getY());
        fadeInPath.lineTo(bounds.getX(), bounds.getY() + maxHeight);
        fadeInPath.closeSubPath();

        // Semi-transparent fill
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.fillPath(fadeInPath);

        // Stroke
        g.setColour(Theme::Colours::accent().withAlpha(0.8f));
        g.strokePath(fadeInPath, juce::PathStrokeType(1.5f));

        // Handle circle at apex
        float handleX = bounds.getX() + actualWidth;
        float handleY = bounds.getY() + 4;
        auto handleColor = (hoveredFade == FadeHover::FadeIn || activeFade == FadeHover::FadeIn)
                             ? Theme::Colours::accent()
                             : Theme::Colours::accent().withAlpha(0.6f);

        g.setColour(handleColor);
        g.fillEllipse(handleX - fadeHandleSize / 2, handleY,
                      static_cast<float>(fadeHandleSize), static_cast<float>(fadeHandleSize));

        // White center dot
        g.setColour(juce::Colours::white);
        g.fillEllipse(handleX - 2, handleY + 2, 4, 4);
    }

    // Fade Out triangle (top-right corner)
    if (fadeOutWidth >= 2.0f || hoveredFade == FadeHover::FadeOut || draggingFade)
    {
        float actualWidth = juce::jmax(fadeOutWidth, 4.0f);

        juce::Path fadeOutPath;
        fadeOutPath.startNewSubPath(bounds.getRight(), bounds.getY());
        fadeOutPath.lineTo(bounds.getRight() - actualWidth, bounds.getY());
        fadeOutPath.lineTo(bounds.getRight(), bounds.getY() + maxHeight);
        fadeOutPath.closeSubPath();

        // Semi-transparent fill
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.fillPath(fadeOutPath);

        // Stroke
        g.setColour(Theme::Colours::accent().withAlpha(0.8f));
        g.strokePath(fadeOutPath, juce::PathStrokeType(1.5f));

        // Handle circle at apex
        float handleX = bounds.getRight() - actualWidth;
        float handleY = bounds.getY() + 4;
        auto handleColor = (hoveredFade == FadeHover::FadeOut || activeFade == FadeHover::FadeOut)
                             ? Theme::Colours::accent()
                             : Theme::Colours::accent().withAlpha(0.6f);

        g.setColour(handleColor);
        g.fillEllipse(handleX - fadeHandleSize / 2, handleY,
                      static_cast<float>(fadeHandleSize), static_cast<float>(fadeHandleSize));

        // White center dot
        g.setColour(juce::Colours::white);
        g.fillEllipse(handleX - 2, handleY + 2, 4, 4);
    }
}

bool ClipComponent::hitTestFadeHandle(const juce::Point<int>& pos, FadeHover& which) const
{
    const float fadeInWidth = static_cast<float>(timeToPixels(clipData.fadeInDuration));
    const float fadeOutWidth = static_cast<float>(timeToPixels(clipData.fadeOutDuration));

    // Fade In handle (top-left area, near the apex)
    float fadeInHandleX = juce::jmax(fadeInWidth, 4.0f);
    if (pos.x >= fadeInHandleX - fadeHitZone && pos.x <= fadeInHandleX + fadeHitZone / 2 &&
        pos.y < 20)  // Top area only
    {
        which = FadeHover::FadeIn;
        return true;
    }

    // Fade Out handle (top-right area, near the apex)
    float fadeOutHandleX = static_cast<float>(getWidth()) - juce::jmax(fadeOutWidth, 4.0f);
    if (pos.x >= fadeOutHandleX - fadeHitZone / 2 && pos.x <= fadeOutHandleX + fadeHitZone &&
        pos.y < 20)  // Top area only
    {
        which = FadeHover::FadeOut;
        return true;
    }

    which = FadeHover::None;
    return false;
}

void ClipComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(18);  // Space for name
    bounds.reduce(2, 2);
    waveformDisplay.setBounds(bounds);
}

void ClipComponent::setClipData(const Clip& clip)
{
    clipData = clip;
    waveformDisplay.setVisibleRange(clip.sourceStartOffset,
                                     clip.sourceStartOffset + clip.duration);

    // Update waveform gradient colors based on clip color
    auto topColour = Theme::TrackColours::getWaveformTop(clip.colour);
    auto bottomColour = Theme::TrackColours::getWaveformBottom(clip.colour);
    waveformDisplay.setGradientColours(topColour, bottomColour);
    waveformDisplay.setBackgroundColour(clip.colour.darker(0.6f).withAlpha(0.3f));

    // Set tooltip with name and duration
    juce::String tooltip = clip.name + " (" + juce::String(clip.duration, 1) + "s)";
    setTooltip(tooltip);

    repaint();
}

void ClipComponent::setThumbnail(juce::AudioThumbnail* thumb)
{
    waveformDisplay.setThumbnail(thumb);

    // Set gradient colors based on clip color using TrackColours helpers
    auto topColour = Theme::TrackColours::getWaveformTop(clipData.colour);
    auto bottomColour = Theme::TrackColours::getWaveformBottom(clipData.colour);
    waveformDisplay.setGradientColours(topColour, bottomColour);
    waveformDisplay.setWaveformColour(clipData.colour.brighter(0.3f));
}

void ClipComponent::setSelected(bool sel)
{
    if (selected != sel)
    {
        selected = sel;
        repaint();
    }
}

void ClipComponent::mouseDown(const juce::MouseEvent& e)
{
    // Notify selection with modifier info for multi-select
    if (onSelect)
        onSelect(this, e.mods.isCtrlDown());

    // Context menu on right-click
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Delete");
        menu.addItem(2, "Duplicate");
        menu.addSeparator();
        menu.addItem(3, clipData.name, false);  // Show name (disabled)

        menu.showMenuAsync(juce::PopupMenu::Options(),
            [this](int result)
            {
                if (result == 1 && onDelete)
                    onDelete(this);
                else if (result == 2 && onDuplicate)
                    onDuplicate(this);
            });
        return;
    }

    dragStart = e.getPosition();
    dragStartScreen = e.getScreenPosition();
    verticalDragActive = false;

    // Check if we're starting a fade drag operation (priority)
    if (hoveredFade != FadeHover::None)
    {
        draggingFade = true;
        activeFade = hoveredFade;
        trimming = false;
        dragging = false;

        // Store initial fade duration
        if (activeFade == FadeHover::FadeIn)
            fadeStartValue = clipData.fadeInDuration;
        else
            fadeStartValue = clipData.fadeOutDuration;
    }
    // Check if we're starting a trim operation
    else if (hoveredEdge != EdgeHover::None)
    {
        trimming = true;
        trimEdge = hoveredEdge;
        draggingFade = false;
        dragging = false;

        // Store initial value for trimming
        if (trimEdge == EdgeHover::Left)
            trimStartValue = clipData.startTime;
        else
            trimStartValue = clipData.duration;
    }
    else
    {
        trimming = false;
        draggingFade = false;
        dragging = true;

        // Store Y center of parent track for vertical offset calculation
        if (auto* parent = getParentComponent())
            initialTrackY = parent->getScreenY() + parent->getHeight() / 2;

        // Notify drag start
        if (onDragStart)
            onDragStart(this);
    }
}

void ClipComponent::mouseDrag(const juce::MouseEvent& e)
{
    int deltaX = e.getPosition().x - dragStart.x;

    // Handle fade drag operation
    if (draggingFade)
    {
        if (std::abs(deltaX) > 1)
        {
            double deltaTime = pixelsToTime(static_cast<double>(deltaX));
            double newFadeDuration = 0.0;

            if (activeFade == FadeHover::FadeIn)
            {
                // Fade in: dragging right increases, left decreases
                newFadeDuration = juce::jmax(0.0, clipData.fadeInDuration + deltaTime);
                // Limit to half clip duration
                newFadeDuration = juce::jmin(newFadeDuration, clipData.duration * 0.5);
            }
            else
            {
                // Fade out: dragging left increases, right decreases
                newFadeDuration = juce::jmax(0.0, clipData.fadeOutDuration - deltaTime);
                // Limit to half clip duration
                newFadeDuration = juce::jmin(newFadeDuration, clipData.duration * 0.5);
            }

            if (onFadeChanged)
            {
                bool isFadeIn = (activeFade == FadeHover::FadeIn);
                onFadeChanged(this, isFadeIn, newFadeDuration);
            }

            dragStart = e.getPosition();
            repaint();
        }
        return;
    }

    // Handle trim operation
    if (trimming)
    {
        if (onTrim && std::abs(deltaX) > 1)
        {
            bool isLeftEdge = (trimEdge == EdgeHover::Left);
            onTrim(this, isLeftEdge, static_cast<double>(deltaX));
            dragStart = e.getPosition();
        }
        return;
    }

    // Normal drag operation
    if (!dragging)
        return;

    int screenY = e.getScreenPosition().y;

    // Detect vertical drag using absolute track index
    if (onQueryTrackAtY)
    {
        int targetTrackIndex = onQueryTrackAtY(screenY);

        if (targetTrackIndex >= 0 && targetTrackIndex != currentTrackIndex)
        {
            // Report track change with absolute target index
            if (onDragToNewTrack)
                onDragToNewTrack(this, targetTrackIndex);

            // Show visual feedback on target track
            if (onSetDropTarget)
                onSetDropTarget(targetTrackIndex);

            verticalDragActive = true;
        }
    }

    // Horizontal drag
    if (onDrag && std::abs(deltaX) > 2)
    {
        onDrag(this, static_cast<double>(deltaX));
        dragStart = e.getPosition();
    }
}

void ClipComponent::mouseUp(const juce::MouseEvent& /*e*/)
{
    if (draggingFade)
    {
        draggingFade = false;
        activeFade = FadeHover::None;
    }

    if (trimming)
    {
        trimming = false;
        trimEdge = EdgeHover::None;
    }

    if (dragging)
    {
        dragging = false;

        // Notify drag end
        if (onDragEnd)
            onDragEnd(this);
    }
}

void ClipComponent::mouseEnter(const juce::MouseEvent& /*e*/)
{
    hovered = true;
    repaint();
}

void ClipComponent::mouseMove(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();

    // Check fade handles first (priority over trim)
    FadeHover newFade = FadeHover::None;
    hitTestFadeHandle(pos, newFade);

    if (newFade != hoveredFade)
    {
        hoveredFade = newFade;
        if (newFade != FadeHover::None)
        {
            hoveredEdge = EdgeHover::None;  // Clear trim hover
            updateMouseCursor();
            repaint();
            return;
        }
    }

    // Check trim edges
    EdgeHover newEdge = EdgeHover::None;

    if (pos.x < edgeHitZone)
        newEdge = EdgeHover::Left;
    else if (pos.x > getWidth() - edgeHitZone)
        newEdge = EdgeHover::Right;

    if (newEdge != hoveredEdge)
    {
        hoveredEdge = newEdge;
        updateMouseCursor();
        repaint();  // Visual feedback for trim zones
    }
}

void ClipComponent::mouseExit(const juce::MouseEvent& /*e*/)
{
    hovered = false;
    hoveredEdge = EdgeHover::None;
    hoveredFade = FadeHover::None;
    updateMouseCursor();
    repaint();
}

void ClipComponent::updateMouseCursor()
{
    if (hoveredFade != FadeHover::None)
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else if (hoveredEdge != EdgeHover::None)
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}
