#include "TrackLane.h"
#include "UI/Theme/AppTheme.h"

TrackLane::TrackLane()
{
}

TrackLane::~TrackLane()
{
}

void TrackLane::paint(juce::Graphics& g)
{
    // Track background
    g.fillAll(Theme::colour(Theme::bgSlot));

    // Selection highlight
    if (selected)
    {
        g.setColour(Theme::Colours::accent().withAlpha(0.15f));
        g.fillRect(getLocalBounds());
    }

    // Header area
    auto headerArea = getLocalBounds().removeFromLeft(headerWidth);
    g.setColour(trackData.colour.withAlpha(selected ? 0.5f : 0.3f));
    g.fillRect(headerArea);

    // Track name
    g.setColour(Theme::Colours::text());
    g.setFont(12.0f);
    g.drawText(trackData.name,
               headerArea.reduced(8, 4),
               juce::Justification::centredLeft);

    // Mute/Solo indicators
    auto indicatorArea = headerArea.removeFromBottom(20).reduced(8, 2);
    if (trackData.muted)
    {
        g.setColour(Theme::Colours::error().withAlpha(0.7f));
        g.fillRoundedRectangle(indicatorArea.removeFromLeft(20).toFloat(), Theme::cornerRadiusSm);
        g.setColour(Theme::Colours::text());
        g.setFont(10.0f);
        g.drawText("M", indicatorArea.withWidth(20), juce::Justification::centred);
    }

    if (trackData.solo)
    {
        g.setColour(Theme::colour(Theme::meterMid).withAlpha(0.7f));
        g.fillRoundedRectangle(indicatorArea.removeFromLeft(20).toFloat(), Theme::cornerRadiusSm);
        g.setColour(Theme::colour(Theme::textOnAccent));
        g.setFont(10.0f);
        g.drawText("S", indicatorArea.withWidth(20), juce::Justification::centred);
    }

    // Empty clip state
    if (clipComponents.empty())
    {
        auto contentArea = getLocalBounds().withTrimmedLeft(headerWidth);
        g.setColour(Theme::Colours::textDisabled());
        g.setFont(11.0f);
        g.drawText("Drop audio here", contentArea, juce::Justification::centred);
    }

    // Border
    g.setColour(Theme::colour(Theme::border));
    g.drawRect(getLocalBounds());
}

void TrackLane::resized()
{
    layoutClips();
}

void TrackLane::setTrackData(const Track& track)
{
    trackData = track;
    repaint();
}

void TrackLane::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = pps;
    layoutClips();
}

void TrackLane::setVisibleRange(double startTime, double endTime)
{
    visibleStart = startTime;
    visibleEnd = endTime;
    layoutClips();
}

void TrackLane::addClipComponent(const Clip& clipData, juce::AudioThumbnail* thumbnail)
{
    auto clip = std::make_unique<ClipComponent>();
    clip->setClipData(clipData);

    if (thumbnail != nullptr)
        clip->setThumbnail(thumbnail);

    // Set callbacks
    clip->onSelect = [this](ClipComponent* c)
    {
        if (selectedClip != nullptr)
            selectedClip->setSelected(false);

        selectedClip = c;
        c->setSelected(true);

        if (onClipSelected)
            onClipSelected(this, c);
    };

    clip->onDrag = [this](ClipComponent* c, double deltaPixels)
    {
        double deltaTime = deltaPixels / pixelsPerSecond;
        Clip data = c->getClipData();
        data.startTime = juce::jmax(0.0, data.startTime + deltaTime);
        c->setClipData(data);
        layoutClips();
    };

    addAndMakeVisible(*clip);
    clipComponents.push_back(std::move(clip));

    layoutClips();
}

void TrackLane::clearClips()
{
    clipComponents.clear();
    selectedClip = nullptr;
}

void TrackLane::layoutClips()
{
    auto contentArea = getLocalBounds();
    contentArea.removeFromLeft(headerWidth);

    for (auto& clip : clipComponents)
    {
        const auto& data = clip->getClipData();

        int x = static_cast<int>((data.startTime - visibleStart) * pixelsPerSecond);
        int width = static_cast<int>(data.duration * pixelsPerSecond);

        clip->setBounds(headerWidth + x, 4, width, getHeight() - 8);
    }
}

void TrackLane::mouseDown(const juce::MouseEvent& /*e*/)
{
    // Select this track when clicked anywhere
    if (onTrackSelected)
        onTrackSelected(this);
}

void TrackLane::mouseDoubleClick(const juce::MouseEvent& e)
{
    // Check if double-click is in header area (track name zone)
    auto headerArea = getLocalBounds().removeFromLeft(headerWidth);
    if (headerArea.contains(e.getPosition()))
    {
        showNameEditor();
    }
}

void TrackLane::setSelected(bool shouldBeSelected)
{
    if (selected != shouldBeSelected)
    {
        selected = shouldBeSelected;
        repaint();
    }
}

void TrackLane::showNameEditor()
{
    if (isEditingName)
        return;

    isEditingName = true;

    // Create text editor
    nameEditor = std::make_unique<juce::TextEditor>();
    nameEditor->setMultiLine(false);
    nameEditor->setReturnKeyStartsNewLine(false);
    nameEditor->setText(trackData.name, false);
    nameEditor->selectAll();
    nameEditor->addListener(this);

    // Style
    nameEditor->setColour(juce::TextEditor::backgroundColourId, Theme::colour(Theme::bgSlot));
    nameEditor->setColour(juce::TextEditor::textColourId, Theme::Colours::text());
    nameEditor->setColour(juce::TextEditor::outlineColourId, Theme::Colours::accent());
    nameEditor->setColour(juce::TextEditor::focusedOutlineColourId, Theme::Colours::accent());
    nameEditor->setFont(12.0f);

    // Position over the track name
    auto headerArea = getLocalBounds().removeFromLeft(headerWidth);
    auto nameBounds = headerArea.reduced(4, 4).removeFromTop(24);
    nameEditor->setBounds(nameBounds);

    addAndMakeVisible(*nameEditor);
    nameEditor->grabKeyboardFocus();
}

void TrackLane::hideNameEditor()
{
    if (nameEditor != nullptr)
    {
        nameEditor->removeListener(this);
        removeChildComponent(nameEditor.get());
        nameEditor.reset();
    }
    isEditingName = false;
    repaint();
}

void TrackLane::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    auto newName = editor.getText().trim();
    if (newName.isNotEmpty() && newName != trackData.name)
    {
        trackData.name = newName;
        if (onTrackRenamed)
            onTrackRenamed(this, newName);
    }
    hideNameEditor();
}

void TrackLane::textEditorEscapeKeyPressed(juce::TextEditor& /*editor*/)
{
    hideNameEditor();
}

void TrackLane::textEditorFocusLost(juce::TextEditor& editor)
{
    // Treat focus loss as confirmation
    textEditorReturnKeyPressed(editor);
}
