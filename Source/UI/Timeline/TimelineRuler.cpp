#include "TimelineRuler.h"

TimelineRuler::TimelineRuler()
{
}

void TimelineRuler::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff252525));

    drawTimeTicks(g);
    drawPlayhead(g);

    // Bottom border
    g.setColour(juce::Colour(0xff3a3a3a));
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
        float x = static_cast<float>((t - visibleStart) * pixelsPerSecond);

        bool isMajor = std::fmod(t, tickInterval * 4) < 0.001;

        if (isMajor)
        {
            // Major tick
            g.setColour(juce::Colour(0xff808080));
            g.drawVerticalLine(static_cast<int>(x), height * 0.3f, height);

            // Time label
            int minutes = static_cast<int>(t) / 60;
            int seconds = static_cast<int>(t) % 60;
            g.setColour(juce::Colours::lightgrey);
            g.drawText(juce::String::formatted("%d:%02d", minutes, seconds),
                       static_cast<int>(x) - 20, 2, 40, 14,
                       juce::Justification::centred);
        }
        else
        {
            // Minor tick
            g.setColour(juce::Colour(0xff505050));
            g.drawVerticalLine(static_cast<int>(x), height * 0.6f, height);
        }
    }
}

void TimelineRuler::drawPlayhead(juce::Graphics& g)
{
    if (playheadPosition < visibleStart || playheadPosition > visibleEnd)
        return;

    float x = static_cast<float>((playheadPosition - visibleStart) * pixelsPerSecond);

    // Playhead triangle
    juce::Path triangle;
    triangle.addTriangle(x - 6, 0, x + 6, 0, x, 10);

    g.setColour(juce::Colour(0xffff5722));
    g.fillPath(triangle);

    // Vertical line
    g.drawVerticalLine(static_cast<int>(x), 10.0f, static_cast<float>(getHeight()));
}
