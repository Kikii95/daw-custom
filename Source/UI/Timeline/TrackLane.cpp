#include "TrackLane.h"
#include "UI/Theme/AppTheme.h"
#include <limits>

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

    // Drop target highlight (for drag-drop feedback)
    if (dropTarget)
    {
        auto contentArea = getLocalBounds().withTrimmedLeft(headerWidth);
        g.setColour(Theme::Colours::accent().withAlpha(0.25f));
        g.fillRect(contentArea);

        // Dashed border effect
        g.setColour(Theme::Colours::accent());
        g.drawRect(contentArea.reduced(2), 2);
    }

    // Header area
    auto headerArea = getLocalBounds().removeFromLeft(headerWidth);
    g.setColour(trackData.colour.withAlpha(selected ? 0.5f : (dropTarget ? 0.6f : 0.3f)));
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

    // Empty clip state (only show if not drop target)
    if (clipComponents.empty() && !dropTarget)
    {
        auto contentArea = getLocalBounds().withTrimmedLeft(headerWidth);
        g.setColour(Theme::Colours::textDisabled());
        g.setFont(11.0f);
        g.drawText("Drop audio here", contentArea, juce::Justification::centred);
    }

    // Border
    g.setColour(dropTarget ? Theme::Colours::accent() : Theme::colour(Theme::border));
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

    // Propagate to all clip components for fade handle calculations
    for (auto& clip : clipComponents)
        clip->setPixelsPerSecond(pps);

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
    clip->onSelect = [this](ClipComponent* c, bool addToSelection)
    {
        // If not adding to selection, deselect previous
        if (!addToSelection && selectedClip != nullptr && selectedClip != c)
            selectedClip->setSelected(false);

        // If adding to selection and already selected, toggle off
        if (addToSelection && c->isSelected())
        {
            c->setSelected(false);
            if (selectedClip == c)
                selectedClip = nullptr;
        }
        else
        {
            selectedClip = c;
            c->setSelected(true);
        }

        if (onClipSelected)
            onClipSelected(this, c, addToSelection);
    };

    clip->onDrag = [this](ClipComponent* c, double deltaPixels)
    {
        // Notify TimelinePanel for potential multi-clip drag
        if (onClipDragDelta)
        {
            onClipDragDelta(this, c, deltaPixels);
        }
        else
        {
            // Fallback: single clip drag (direct modification)
            double deltaTime = deltaPixels / pixelsPerSecond;
            Clip data = c->getClipData();
            double newTime = juce::jmax(0.0, data.startTime + deltaTime);

            bool shiftHeld = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
            if (snapEnabled && !shiftHeld)
                newTime = snapToGrid(newTime);

            data.startTime = newTime;
            c->setClipData(data);
            layoutClips();
        }
    };

    // Set current track index for the clip
    clip->currentTrackIndex = trackIndex;

    clip->onDragToNewTrack = [this](ClipComponent* c, int targetTrackIndex)
    {
        // Now passes absolute target track index
        if (onClipDraggedToTrack)
            onClipDraggedToTrack(this, c, targetTrackIndex);
    };

    // Query track index at screen Y position
    clip->onQueryTrackAtY = [this](int screenY) -> int
    {
        if (onQueryTrackAtScreenY)
            return onQueryTrackAtScreenY(screenY);
        return -1;
    };

    // Visual feedback for drop target
    clip->onSetDropTarget = [this](int targetTrackIndex)
    {
        if (onSetDropTargetTrack)
            onSetDropTargetTrack(targetTrackIndex);
    };

    clip->onDelete = [this](ClipComponent* c)
    {
        if (onClipDelete)
            onClipDelete(this, c);
    };

    clip->onDuplicate = [this](ClipComponent* c)
    {
        if (onClipDuplicate)
            onClipDuplicate(this, c);
    };

    clip->onDragStart = [this](ClipComponent* c)
    {
        if (onClipDragStart)
            onClipDragStart(this, c);
    };

    clip->onDragEnd = [this](ClipComponent* c)
    {
        if (onClipDragEnd)
            onClipDragEnd(this, c);
    };

    clip->onTrim = [this](ClipComponent* c, bool isLeftEdge, double deltaPixels)
    {
        if (onClipTrim)
        {
            onClipTrim(this, c, isLeftEdge, deltaPixels);
        }
        else
        {
            // Fallback: direct modification
            double deltaTime = deltaPixels / pixelsPerSecond;
            Clip data = c->getClipData();

            bool shiftHeld = juce::ModifierKeys::getCurrentModifiers().isShiftDown();

            if (isLeftEdge)
            {
                // Trim left: adjust startTime, sourceStartOffset, duration
                double newStart = juce::jmax(0.0, data.startTime + deltaTime);
                if (snapEnabled && !shiftHeld)
                    newStart = snapToGrid(newStart);

                double startDelta = newStart - data.startTime;
                double newDuration = data.duration - startDelta;

                // Prevent trimming past end or into negative sourceOffset
                if (newDuration > 0.1 && (data.sourceStartOffset + startDelta) >= 0.0)
                {
                    data.startTime = newStart;
                    data.sourceStartOffset += startDelta;
                    data.duration = newDuration;
                }
            }
            else
            {
                // Trim right: adjust duration only
                double newDuration = juce::jmax(0.1, data.duration + deltaTime);
                if (snapEnabled && !shiftHeld)
                {
                    double newEnd = data.startTime + newDuration;
                    newEnd = snapToGrid(newEnd);
                    newDuration = newEnd - data.startTime;
                }
                data.duration = juce::jmax(0.1, newDuration);
            }

            c->setClipData(data);
            layoutClips();
        }
    };

    clip->onFadeChanged = [this](ClipComponent* c, bool isFadeIn, double newDuration)
    {
        if (onClipFadeChanged)
        {
            onClipFadeChanged(this, c, isFadeIn, newDuration);
        }
        else
        {
            // Fallback: direct modification
            Clip data = c->getClipData();
            if (isFadeIn)
                data.fadeInDuration = newDuration;
            else
                data.fadeOutDuration = newDuration;
            c->setClipData(data);
        }
    };

    // Set pixels per second for fade handle calculations
    clip->setPixelsPerSecond(pixelsPerSecond);

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

void TrackLane::mouseDown(const juce::MouseEvent& e)
{
    // Select this track when clicked anywhere
    if (onTrackSelected)
        onTrackSelected(this);

    // Context menu on right-click in header
    auto headerArea = getLocalBounds().removeFromLeft(headerWidth);
    if (e.mods.isPopupMenu() && headerArea.contains(e.getPosition()))
    {
        juce::PopupMenu colourMenu;
        for (int i = 0; i < 16; ++i)
        {
            auto colour = Theme::TrackColours::getColour(i);
            colourMenu.addItem(100 + i, Theme::TrackColours::getName(i));
        }

        juce::PopupMenu menu;
        menu.addItem(1, "Rename");
        menu.addSubMenu("Track Colour", colourMenu);
        menu.addSeparator();
        menu.addItem(2, "Delete Track");

        menu.showMenuAsync(juce::PopupMenu::Options(),
            [this](int result)
            {
                if (result == 1)
                    showNameEditor();
                else if (result == 2 && onTrackDelete)
                    onTrackDelete(this);
                else if (result >= 100 && result < 116 && onTrackColourChanged)
                    onTrackColourChanged(this, Theme::TrackColours::getColour(result - 100));
            });
    }
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

void TrackLane::setDropTarget(bool isTarget)
{
    if (dropTarget != isTarget)
    {
        dropTarget = isTarget;
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

double TrackLane::snapToGrid(double time) const
{
    if (!snapEnabled || snapInterval <= 0.0)
        return time;

    double snappedTime = time;
    double minDist = std::numeric_limits<double>::max();
    constexpr double clipEdgeSnapThreshold = 0.1;  // 100ms threshold for clip edge snap

    // 1. Grid snap
    double gridSnapped = std::round(time / snapInterval) * snapInterval;
    double gridDist = std::abs(time - gridSnapped);
    if (gridDist < minDist)
    {
        minDist = gridDist;
        snappedTime = gridSnapped;
    }

    // 2. Clip edge snap (start and end of other clips on this track)
    for (const auto& clipComp : clipComponents)
    {
        const auto& clip = clipComp->getClipData();

        // Snap to clip start
        double startDist = std::abs(time - clip.startTime);
        if (startDist < minDist && startDist < clipEdgeSnapThreshold)
        {
            minDist = startDist;
            snappedTime = clip.startTime;
        }

        // Snap to clip end
        double clipEndTime = clip.getEndTime();
        double endDist = std::abs(time - clipEndTime);
        if (endDist < minDist && endDist < clipEdgeSnapThreshold)
        {
            minDist = endDist;
            snappedTime = clipEndTime;
        }
    }

    return snappedTime;
}
