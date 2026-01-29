#include "OscilloscopePanel.h"
#include "UI/Theme/AppTheme.h"

OscilloscopePanel::OscilloscopePanel()
    : bufferL(BUFFER_SIZE, 0.0f),
      bufferR(BUFFER_SIZE, 0.0f)
{
    startTimerHz(30);  // 30 FPS
}

OscilloscopePanel::~OscilloscopePanel()
{
    stopTimer();
}

void OscilloscopePanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.fillAll(Theme::colour(Theme::bgDark));

    // Border
    g.setColour(Theme::colour(Theme::border));
    g.drawRect(bounds, 1.0f);

    // Inner drawing area
    auto drawBounds = bounds.reduced(2);

    // Draw grid
    drawGrid(g, drawBounds);

    // Draw waveforms
    if (showStereo)
    {
        // Split view: L top, R bottom
        auto topHalf = drawBounds.removeFromTop(drawBounds.getHeight() * 0.5f);
        auto bottomHalf = drawBounds;

        drawWaveform(g, topHalf, bufferL, leftColour);
        drawWaveform(g, bottomHalf, bufferR, rightColour);

        // Divider line
        g.setColour(gridColour);
        g.drawHorizontalLine(static_cast<int>(topHalf.getBottom()),
                             topHalf.getX(), topHalf.getRight());

        // Channel labels
        g.setColour(leftColour.withAlpha(0.7f));
        g.setFont(10.0f);
        g.drawText("L", topHalf.reduced(4, 2), juce::Justification::topLeft);
        g.setColour(rightColour.withAlpha(0.7f));
        g.drawText("R", bottomHalf.reduced(4, 2), juce::Justification::topLeft);
    }
    else
    {
        // Mono view: show left channel only
        drawWaveform(g, drawBounds, bufferL, leftColour);
    }

    // Frozen indicator
    if (frozen)
    {
        g.setColour(Theme::Colours::warning().withAlpha(0.8f));
        g.setFont(12.0f);
        g.drawText("FROZEN", bounds.reduced(4), juce::Justification::topRight);
    }
}

void OscilloscopePanel::resized()
{
}

void OscilloscopePanel::timerCallback()
{
    if (!frozen)
        repaint();
}

void OscilloscopePanel::pushSamples(const float* samples, int numSamples)
{
    if (frozen)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        int idx = writeIndex.load();
        bufferL[static_cast<size_t>(idx)] = samples[i];
        bufferR[static_cast<size_t>(idx)] = samples[i];  // Mono to both
        writeIndex.store((idx + 1) % BUFFER_SIZE);
    }
}

void OscilloscopePanel::pushStereoSamples(const float* left, const float* right, int numSamples)
{
    if (frozen)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        int idx = writeIndex.load();
        bufferL[static_cast<size_t>(idx)] = left[i];
        bufferR[static_cast<size_t>(idx)] = right[i];
        writeIndex.store((idx + 1) % BUFFER_SIZE);
    }
}

void OscilloscopePanel::drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(gridColour);

    // Horizontal center line(s)
    if (showStereo)
    {
        float quarterH = bounds.getHeight() * 0.25f;
        g.drawHorizontalLine(static_cast<int>(bounds.getY() + quarterH),
                             bounds.getX(), bounds.getRight());
        g.drawHorizontalLine(static_cast<int>(bounds.getY() + quarterH * 3),
                             bounds.getX(), bounds.getRight());
    }
    else
    {
        g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()),
                             bounds.getX(), bounds.getRight());
    }

    // Vertical divisions (time)
    int numDivisions = 8;
    for (int i = 1; i < numDivisions; ++i)
    {
        float x = bounds.getX() + (bounds.getWidth() * i / numDivisions);
        g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }
}

void OscilloscopePanel::drawWaveform(juce::Graphics& g, juce::Rectangle<float> bounds,
                                      const std::vector<float>& buffer, juce::Colour colour)
{
    if (buffer.empty() || bounds.getWidth() < 2)
        return;

    // Calculate how many samples to display
    int samplesToShow = juce::jmin(static_cast<int>(buffer.size()),
                                    static_cast<int>(bounds.getWidth()));

    // Start from write position going backwards
    int startIdx = (writeIndex.load() - samplesToShow + BUFFER_SIZE) % BUFFER_SIZE;

    juce::Path path;
    bool pathStarted = false;

    float centreY = bounds.getCentreY();
    float amplitude = bounds.getHeight() * 0.45f * displayGain;

    for (int i = 0; i < samplesToShow; ++i)
    {
        int bufIdx = (startIdx + i) % BUFFER_SIZE;
        float sample = buffer[static_cast<size_t>(bufIdx)];

        // Clamp to prevent drawing outside bounds
        sample = juce::jlimit(-1.0f, 1.0f, sample);

        float x = bounds.getX() + (i * bounds.getWidth() / samplesToShow);
        float y = centreY - (sample * amplitude);

        if (!pathStarted)
        {
            path.startNewSubPath(x, y);
            pathStarted = true;
        }
        else
        {
            path.lineTo(x, y);
        }
    }

    // Draw waveform with glow effect
    g.setColour(colour.withAlpha(0.3f));
    g.strokePath(path, juce::PathStrokeType(3.0f));

    g.setColour(colour);
    g.strokePath(path, juce::PathStrokeType(1.5f));
}
