#include "WaveformDisplay.h"
#include "UI/Theme/AppTheme.h"

WaveformDisplay::WaveformDisplay()
{
}

void WaveformDisplay::setGradientColours(juce::Colour top, juce::Colour bottom)
{
    gradientTopColour = top;
    gradientBottomColour = bottom;
    repaint();
}

WaveformDisplay::~WaveformDisplay()
{
    if (thumbnail != nullptr)
        thumbnail->removeChangeListener(this);
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColour);

    if (thumbnail == nullptr || thumbnail->getTotalLength() <= 0)
    {
        // Draw placeholder
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawText("No audio loaded", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto bounds = getLocalBounds().toFloat();

    if (useGradient)
    {
        drawGradientWaveform(g, bounds);
    }
    else
    {
        // Legacy: single-color waveform
        g.setColour(waveformColour);
        thumbnail->drawChannels(g, bounds.toNearestInt(),
                                visibleStart, visibleEnd,
                                1.0f);
    }

    // Draw center line
    g.setColour(waveformColour.withAlpha(0.2f));
    g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()),
                         bounds.getX(), bounds.getRight());
}

void WaveformDisplay::drawGradientWaveform(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const int numSamples = static_cast<int>(bounds.getWidth());
    if (numSamples <= 0)
        return;

    // Pre-allocate vectors for min/max levels
    std::vector<float> minLevels(static_cast<size_t>(numSamples));
    std::vector<float> maxLevels(static_cast<size_t>(numSamples));

    // Calculate time step per pixel
    double timeRange = visibleEnd - visibleStart;
    double timeStep = timeRange / numSamples;

    // Get min/max levels from thumbnail for each pixel column
    for (int i = 0; i < numSamples; ++i)
    {
        double startTime = visibleStart + i * timeStep;
        double endTime = startTime + timeStep;

        float minVal = 0.0f, maxVal = 0.0f;
        thumbnail->getApproximateMinMax(startTime, endTime, 0, minVal, maxVal);

        minLevels[static_cast<size_t>(i)] = minVal;
        maxLevels[static_cast<size_t>(i)] = maxVal;
    }

    float centreY = bounds.getCentreY();
    float halfHeight = bounds.getHeight() * 0.5f;

    // Build waveform path
    juce::Path waveformPath;

    // Start at first point (top of waveform)
    float x = bounds.getX();
    float topY = centreY - maxLevels[0] * halfHeight;
    waveformPath.startNewSubPath(x, topY);

    // Draw upper edge (max levels)
    for (int i = 1; i < numSamples; ++i)
    {
        x = bounds.getX() + static_cast<float>(i);
        topY = centreY - maxLevels[static_cast<size_t>(i)] * halfHeight;
        waveformPath.lineTo(x, topY);
    }

    // Draw lower edge in reverse (min levels)
    for (int i = numSamples - 1; i >= 0; --i)
    {
        x = bounds.getX() + static_cast<float>(i);
        float bottomY = centreY - minLevels[static_cast<size_t>(i)] * halfHeight;
        waveformPath.lineTo(x, bottomY);
    }

    waveformPath.closeSubPath();

    // Vertical gradient fill
    juce::ColourGradient waveGradient(
        gradientTopColour, bounds.getX(), bounds.getY(),
        gradientBottomColour, bounds.getX(), bounds.getBottom(),
        false);

    g.setGradientFill(waveGradient);
    g.fillPath(waveformPath);

    // Subtle outline glow
    g.setColour(gradientTopColour.withAlpha(0.35f));
    g.strokePath(waveformPath, juce::PathStrokeType(0.75f));

    // Inner highlight on top peaks
    juce::Path highlightPath;
    highlightPath.startNewSubPath(bounds.getX(), centreY - maxLevels[0] * halfHeight);
    for (int i = 1; i < numSamples; ++i)
    {
        x = bounds.getX() + static_cast<float>(i);
        topY = centreY - maxLevels[static_cast<size_t>(i)] * halfHeight;
        highlightPath.lineTo(x, topY);
    }

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.strokePath(highlightPath, juce::PathStrokeType(1.0f));
}

void WaveformDisplay::setThumbnail(juce::AudioThumbnail* thumb)
{
    if (thumbnail != nullptr)
        thumbnail->removeChangeListener(this);

    thumbnail = thumb;

    if (thumbnail != nullptr)
    {
        thumbnail->addChangeListener(this);
        visibleEnd = thumbnail->getTotalLength();
    }

    repaint();
}

void WaveformDisplay::setVisibleRange(double startTime, double endTime)
{
    visibleStart = startTime;
    visibleEnd = endTime;
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == thumbnail)
        repaint();
}
