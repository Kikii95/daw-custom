#include "TimelineRuler.h"
#include "UI/Theme/AppTheme.h"

TimelineRuler::TimelineRuler()
{
}

void TimelineRuler::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(Theme::colour(Theme::bgPanel));

    // Draw grid: beats or time-based
    if (showBeats)
        drawBeatTicks(g);
    else
        drawTimeTicks(g);

    drawLoopMarkers(g);
    drawPlayhead(g);

    // Bottom border
    g.setColour(Theme::colour(Theme::border));
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TimelineRuler::setVisibleRange(double startTime, double endTime)
{
    visibleStart = startTime;
    visibleEnd = endTime;
    repaint();
}

void TimelineRuler::setPlayheadPosition(double timeInSeconds)
{
    playheadPosition = timeInSeconds;
    repaint();
}

void TimelineRuler::drawTimeTicks(juce::Graphics& g)
{
    float height = static_cast<float>(getHeight());

    // Calculate tick interval based on zoom level
    double tickInterval = 1.0;  // 1 second
    if (pixelsPerSecond < 20) tickInterval = 5.0;
    if (pixelsPerSecond < 10) tickInterval = 10.0;
    if (pixelsPerSecond > 100) tickInterval = 0.5;
    if (pixelsPerSecond > 200) tickInterval = 0.25;

    // Draw ticks
    double startTick = std::floor(visibleStart / tickInterval) * tickInterval;

    g.setFont(10.0f);

    for (double t = startTick; t <= visibleEnd; t += tickInterval)
    {
        // Apply header offset so time 0:00 aligns with clip start
        float x = static_cast<float>(headerOffset + (t - visibleStart) * pixelsPerSecond);

        // Skip if outside visible area
        if (x < headerOffset - 40)
            continue;

        bool isMajor = std::fmod(t, tickInterval * 4) < 0.001;

        if (isMajor)
        {
            // Major tick
            g.setColour(Theme::Colours::textMuted());
            g.drawVerticalLine(static_cast<int>(x), height * 0.3f, height);

            // Time label
            int minutes = static_cast<int>(t) / 60;
            int seconds = static_cast<int>(t) % 60;
            g.setColour(Theme::Colours::text());
            g.drawText(juce::String::formatted("%d:%02d", minutes, seconds),
                       static_cast<int>(x) - 20, 2, 40, 14,
                       juce::Justification::centred);
        }
        else
        {
            // Minor tick
            g.setColour(Theme::colour(Theme::borderLight));
            g.drawVerticalLine(static_cast<int>(x), height * 0.6f, height);
        }
    }
}

void TimelineRuler::drawBeatTicks(juce::Graphics& g)
{
    float height = static_cast<float>(getHeight());

    // Calculate beat and bar durations
    double beatDuration = 60.0 / tempo;  // seconds per beat
    double barDuration = beatDuration * timeSignatureNum;

    // Determine subdivisions based on zoom level
    // At low zoom, show bars only; at high zoom, show 1/16 notes
    int subdivisionsPerBeat = 1;
    if (pixelsPerSecond * beatDuration > 30)
        subdivisionsPerBeat = 2;  // 1/8 notes
    if (pixelsPerSecond * beatDuration > 60)
        subdivisionsPerBeat = 4;  // 1/16 notes

    double subdivisionDuration = beatDuration / subdivisionsPerBeat;

    // Find starting bar
    double startBar = std::floor(visibleStart / barDuration);
    double startTime = startBar * barDuration;

    g.setFont(10.0f);

    // Draw subdivisions, beats, and bars
    for (double t = startTime; t <= visibleEnd + barDuration; t += subdivisionDuration)
    {
        if (t < 0) continue;

        float x = static_cast<float>(headerOffset + (t - visibleStart) * pixelsPerSecond);

        // Skip if outside visible area
        if (x < headerOffset - 40 || x > getWidth() + 40)
            continue;

        // Determine tick type
        double timeInBar = std::fmod(t + 0.0001, barDuration);  // Small offset to avoid floating point issues
        double timeInBeat = std::fmod(t + 0.0001, beatDuration);

        bool isBarLine = timeInBar < 0.001;
        bool isBeatLine = timeInBeat < 0.001;

        if (isBarLine)
        {
            // Bar line (thickest, with label)
            int barNum = static_cast<int>(std::round(t / barDuration)) + 1;

            g.setColour(Theme::Colours::text());
            g.drawVerticalLine(static_cast<int>(x), height * 0.2f, height);

            // Bar number label
            g.drawText(juce::String(barNum),
                       static_cast<int>(x) + 4, 2, 30, 14,
                       juce::Justification::left);
        }
        else if (isBeatLine)
        {
            // Beat line (medium)
            g.setColour(Theme::Colours::textMuted().withAlpha(0.7f));
            g.drawVerticalLine(static_cast<int>(x), height * 0.45f, height);
        }
        else
        {
            // Subdivision line (light)
            g.setColour(Theme::colour(Theme::borderLight).withAlpha(0.5f));
            g.drawVerticalLine(static_cast<int>(x), height * 0.7f, height);
        }
    }
}

void TimelineRuler::drawPlayhead(juce::Graphics& g)
{
    if (playheadPosition < visibleStart || playheadPosition > visibleEnd)
        return;

    // Apply header offset to align with clips
    float x = static_cast<float>(headerOffset + (playheadPosition - visibleStart) * pixelsPerSecond);

    // Playhead triangle
    juce::Path triangle;
    triangle.addTriangle(x - 6, 0, x + 6, 0, x, 10);

    g.setColour(Theme::Colours::warning());
    g.fillPath(triangle);

    // Vertical line
    g.drawVerticalLine(static_cast<int>(x), 10.0f, static_cast<float>(getHeight()));
}

void TimelineRuler::drawLoopMarkers(juce::Graphics& g)
{
    if (!loopEnabled || loopEnd <= loopStart)
        return;

    float height = static_cast<float>(getHeight());
    int xStart = timeToX(loopStart);
    int xEnd = timeToX(loopEnd);

    // Skip if completely outside visible area
    if (xEnd < headerOffset || xStart > getWidth())
        return;

    // Clamp to visible area
    xStart = juce::jmax(headerOffset, xStart);
    xEnd = juce::jmin(getWidth(), xEnd);

    // Loop region background (semi-transparent)
    auto loopColour = Theme::Colours::accent();
    g.setColour(loopColour.withAlpha(0.15f));
    g.fillRect(xStart, 0, xEnd - xStart, static_cast<int>(height));

    // Loop start marker (left bracket)
    g.setColour(loopColour);
    g.fillRect(xStart, 0, 3, static_cast<int>(height));

    // Loop start triangle
    juce::Path startTriangle;
    float sx = static_cast<float>(xStart);
    startTriangle.addTriangle(sx, 0, sx + 8, 0, sx, 8);
    g.fillPath(startTriangle);

    // Loop end marker (right bracket)
    g.fillRect(xEnd - 3, 0, 3, static_cast<int>(height));

    // Loop end triangle
    juce::Path endTriangle;
    float ex = static_cast<float>(xEnd);
    endTriangle.addTriangle(ex, 0, ex - 8, 0, ex, 8);
    g.fillPath(endTriangle);

    // Top bar connecting markers
    g.fillRect(xStart, 0, xEnd - xStart, 2);
}

void TimelineRuler::setLoopEnabled(bool enabled)
{
    loopEnabled = enabled;
    repaint();
}

void TimelineRuler::setLoopRange(double start, double end)
{
    loopStart = start;
    loopEnd = end;
    repaint();
}

double TimelineRuler::xToTime(int x) const
{
    return visibleStart + static_cast<double>(x - headerOffset) / pixelsPerSecond;
}

int TimelineRuler::timeToX(double time) const
{
    return headerOffset + static_cast<int>((time - visibleStart) * pixelsPerSecond);
}

TimelineRuler::DragTarget TimelineRuler::hitTestLoopMarker(int x) const
{
    if (!loopEnabled || loopEnd <= loopStart)
        return DragTarget::None;

    int xStart = timeToX(loopStart);
    int xEnd = timeToX(loopEnd);
    constexpr int hitTolerance = 8;

    // Check if near loop start marker
    if (std::abs(x - xStart) <= hitTolerance)
        return DragTarget::LoopStart;

    // Check if near loop end marker
    if (std::abs(x - xEnd) <= hitTolerance)
        return DragTarget::LoopEnd;

    // Check if inside loop region (for dragging entire region)
    if (x > xStart + hitTolerance && x < xEnd - hitTolerance)
        return DragTarget::LoopRegion;

    return DragTarget::None;
}

void TimelineRuler::mouseDown(const juce::MouseEvent& e)
{
    dragTarget = hitTestLoopMarker(e.x);

    if (dragTarget != DragTarget::None)
    {
        dragStartTime = xToTime(e.x);
        dragLoopStartOffset = loopStart;
    }
    else if (e.mods.isAltDown())
    {
        // Alt+click to set loop start
        double time = xToTime(e.x);
        loopStart = juce::jmax(0.0, time);
        loopEnd = loopStart + 4.0;  // Default 4 second loop
        loopEnabled = true;

        if (onLoopRangeChanged)
            onLoopRangeChanged(loopStart, loopEnd);

        repaint();
    }
    else
    {
        // Click to set playhead position
        double time = xToTime(e.x);
        if (onPositionClicked)
            onPositionClicked(juce::jmax(0.0, time));
    }
}

void TimelineRuler::mouseDrag(const juce::MouseEvent& e)
{
    if (dragTarget == DragTarget::None)
        return;

    double time = xToTime(e.x);
    double delta = time - dragStartTime;

    switch (dragTarget)
    {
        case DragTarget::LoopStart:
            loopStart = juce::jmax(0.0, juce::jmin(time, loopEnd - 0.1));
            break;

        case DragTarget::LoopEnd:
            loopEnd = juce::jmax(loopStart + 0.1, time);
            break;

        case DragTarget::LoopRegion:
        {
            double duration = loopEnd - loopStart;
            double newStart = juce::jmax(0.0, dragLoopStartOffset + delta);
            loopStart = newStart;
            loopEnd = newStart + duration;
            break;
        }

        default:
            break;
    }

    if (onLoopRangeChanged)
        onLoopRangeChanged(loopStart, loopEnd);

    repaint();
}

void TimelineRuler::mouseUp(const juce::MouseEvent&)
{
    dragTarget = DragTarget::None;
}

void TimelineRuler::mouseMove(const juce::MouseEvent& e)
{
    auto target = hitTestLoopMarker(e.x);

    switch (target)
    {
        case DragTarget::LoopStart:
        case DragTarget::LoopEnd:
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            break;

        case DragTarget::LoopRegion:
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            break;

        default:
            setMouseCursor(juce::MouseCursor::NormalCursor);
            break;
    }
}
