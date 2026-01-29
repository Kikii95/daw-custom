#include "MeterComponent.h"
#include "UI/Theme/AppTheme.h"

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
    float height = bounds.getHeight();

    // Convert to dB for display
    float levelDb = linearToDecibels(level);
    float peakDb = linearToDecibels(peak);

    // Clamp to display range (-60 to +6 dB)
    float levelY = decibelsToY(levelDb, height);
    float peakY = decibelsToY(peakDb, height);

    // Gradient fill for level
    juce::ColourGradient gradient(
        Theme::colour(Theme::meterLow),   // Green at bottom
        bounds.getX(), bounds.getBottom(),
        Theme::colour(Theme::meterHigh),  // Red at top
        bounds.getX(), bounds.getY(),
        false
    );
    gradient.addColour(0.7, Theme::colour(Theme::meterMid));  // Yellow in middle

    g.setGradientFill(gradient);
    g.fillRect(bounds.getX(), bounds.getBottom() - levelY,
               bounds.getWidth(), levelY);

    // Peak indicator
    if (peak > 0.001f)
    {
        g.setColour(peak > 1.0f ? Theme::Colours::error() : Theme::Colours::text());
        g.fillRect(bounds.getX(), bounds.getBottom() - peakY - 2,
                   bounds.getWidth(), 2.0f);
    }

    // Scale markers
    g.setColour(Theme::colour(Theme::borderLight));
    for (float db : { 0.0f, -6.0f, -12.0f, -24.0f, -48.0f })
    {
        float y = decibelsToY(db, height);
        g.drawHorizontalLine(static_cast<int>(bounds.getBottom() - y),
                             bounds.getX(), bounds.getRight());
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
