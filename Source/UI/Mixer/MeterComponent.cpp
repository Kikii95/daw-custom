#include "MeterComponent.h"
#include "UI/Theme/AppTheme.h"
#include "UI/Theme/DrawingHelpers.h"

MeterComponent::MeterComponent()
{
    startTimerHz(30);
}

MeterComponent::~MeterComponent()
{
    stopTimer();
}

void MeterComponent::paint(juce::Graphics& g)
{
    g.fillAll(Theme::colour(Theme::meterBg));

    auto bounds = getLocalBounds().toFloat().reduced(2);

    if (stereoMode)
    {
        float halfWidth = bounds.getWidth() / 2.0f - 1.0f;

        auto leftBounds = bounds.removeFromLeft(halfWidth);
        bounds.removeFromLeft(2.0f);  // Gap
        auto rightBounds = bounds;

        drawMeter(g, leftBounds, displayLeft, peakHoldLeft);
        drawMeter(g, rightBounds, displayRight, peakHoldRight);
    }
    else
    {
        drawMeter(g, bounds, displayLeft, peakHoldLeft);
    }

    // Border
    g.setColour(Theme::colour(Theme::border));
    g.drawRect(getLocalBounds());
}

void MeterComponent::setLevel(float level)
{
    leftLevel.store(level);
    rightLevel.store(level);
}

void MeterComponent::setStereo(bool stereo)
{
    stereoMode = stereo;
    repaint();
}

void MeterComponent::setLevels(float left, float right)
{
    leftLevel.store(left);
    rightLevel.store(right);
}

void MeterComponent::timerCallback()
{
    // Get current levels
    float newLeft = leftLevel.load();
    float newRight = rightLevel.load();

    // Smooth decay
    displayLeft = juce::jmax(newLeft, displayLeft * decayRate);
    displayRight = juce::jmax(newRight, displayRight * decayRate);

    // Peak hold
    if (displayLeft > peakHoldLeft)
    {
        peakHoldLeft = displayLeft;
        peakHoldCounterLeft = static_cast<int>(peakHoldTime * 30);  // 30 Hz timer
    }
    else if (peakHoldCounterLeft > 0)
    {
        --peakHoldCounterLeft;
    }
    else
    {
        peakHoldLeft *= 0.95f;
    }

    if (displayRight > peakHoldRight)
    {
        peakHoldRight = displayRight;
        peakHoldCounterRight = static_cast<int>(peakHoldTime * 30);
    }
    else if (peakHoldCounterRight > 0)
    {
        --peakHoldCounterRight;
    }
    else
    {
        peakHoldRight *= 0.95f;
    }

    repaint();
}

void MeterComponent::resetPeak()
{
    peakHoldLeft = 0.0f;
    peakHoldRight = 0.0f;
    peakHoldCounterLeft = 0;
    peakHoldCounterRight = 0;
}

void MeterComponent::drawMeter(juce::Graphics& g, juce::Rectangle<float> bounds,
                                float level, float peak)
{
    const int numSegments = 24;
    const float segmentGap = 1.5f;
    float height = bounds.getHeight();
    float segmentHeight = (height - (numSegments - 1) * segmentGap) / numSegments;

    // Convert to dB for display
    float levelDb = linearToDecibels(level);
    float peakDb = linearToDecibels(peak);

    // Calculate lit segments
    int levelSegments = static_cast<int>((levelDb + 60.0f) / 66.0f * numSegments);
    int peakSegment = static_cast<int>((peakDb + 60.0f) / 66.0f * numSegments);

    levelSegments = juce::jlimit(0, numSegments, levelSegments);
    peakSegment = juce::jlimit(0, numSegments, peakSegment);

    // Draw segments from bottom to top
    for (int i = 0; i < numSegments; ++i)
    {
        float y = bounds.getBottom() - (i + 1) * (segmentHeight + segmentGap) + segmentGap;
        auto segmentBounds = juce::Rectangle<float>(
            bounds.getX(), y, bounds.getWidth(), segmentHeight);

        // Determine segment color based on position
        juce::Colour segmentColour;
        float normalizedPos = static_cast<float>(i) / numSegments;

        if (normalizedPos < 0.6f)
            segmentColour = Theme::colour(Theme::meterLow);    // Green
        else if (normalizedPos < 0.85f)
            segmentColour = Theme::colour(Theme::meterMid);    // Yellow
        else
            segmentColour = Theme::colour(Theme::meterHigh);   // Red

        bool isLit = i < levelSegments;
        bool isPeak = i == peakSegment && peak > 0.001f;

        if (isLit)
        {
            // Lit segment with gradient
            auto gradient = juce::ColourGradient(
                segmentColour.brighter(0.2f), segmentBounds.getX(), segmentBounds.getY(),
                segmentColour.darker(0.1f), segmentBounds.getX(), segmentBounds.getBottom(),
                false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(segmentBounds, 1.5f);

            // Glow for lit segments
            g.setColour(segmentColour.withAlpha(0.25f));
            g.drawRoundedRectangle(segmentBounds.expanded(0.5f), 2.0f, 1.0f);
        }
        else
        {
            // Unlit segment (dim)
            g.setColour(segmentColour.withAlpha(0.12f));
            g.fillRoundedRectangle(segmentBounds, 1.5f);
        }

        // Peak hold indicator
        if (isPeak)
        {
            auto peakColour = peak > 1.0f ? Theme::Colours::error() : Theme::Colours::text();
            g.setColour(peakColour);
            g.fillRoundedRectangle(segmentBounds, 1.5f);

            // Peak glow
            DrawingHelpers::drawGlow(g, segmentBounds, peakColour,
                                      Theme::Glow::radiusSmall, 0.5f);
        }
    }
}

float MeterComponent::linearToDecibels(float linear) const
{
    if (linear <= 0.0f)
        return -60.0f;
    return 20.0f * std::log10(linear);
}

float MeterComponent::decibelsToY(float db, float height) const
{
    // Map -60 to +6 dB range to 0 to height
    float normalised = (db + 60.0f) / 66.0f;
    return juce::jlimit(0.0f, height, normalised * height);
}
