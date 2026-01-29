#include "EffectSlotComponent.h"
#include "UI/Plugins/PluginEditorWindow.h"
#include "UI/Theme/AppTheme.h"
#include "UI/Theme/DrawingHelpers.h"
#include "Audio/DSP/Effects/DelayEffect.h"

EffectSlotComponent::EffectSlotComponent()
{
    nameLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgHover));
    bypassButton.setColour(juce::TextButton::buttonOnColourId, Theme::Colours::warning());
    bypassButton.setColour(juce::TextButton::textColourOffId, Theme::Colours::text());
    bypassButton.setColour(juce::TextButton::textColourOnId, Theme::colour(Theme::textOnAccent));
    bypassButton.setTooltip("Bypass effect (B)");
    bypassButton.onClick = [this]()
    {
        if (effectSlot)
        {
            effectSlot->setBypass(bypassButton.getToggleState());
            if (onBypassChanged)
                onBypassChanged(effectIndex, bypassButton.getToggleState());
        }
    };
    addAndMakeVisible(bypassButton);

    // Edit button - only visible for VST3 plugins
    editButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::accent());
    editButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    editButton.setTooltip("Open plugin editor");
    editButton.onClick = [this]()
    {
        // Open VST3 plugin editor window
        if (isVST3Effect && effectSlot)
        {
            if (auto* vst3Slot = dynamic_cast<VST3EffectSlot*>(effectSlot))
            {
                PluginEditorWindow::getOrCreateWindow(vst3Slot);
            }
        }
        if (onEditClicked)
            onEditClicked(effectIndex);
    };
    editButton.setVisible(false);  // Hidden by default, shown for VST3
    addAndMakeVisible(editButton);

    removeButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::error());
    removeButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    removeButton.setTooltip("Remove effect");
    removeButton.onClick = [this]()
    {
        if (onRemoveClicked)
            onRemoveClicked(effectIndex);
    };
    addAndMakeVisible(removeButton);

    // Preset button
    presetButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgHover));
    presetButton.setColour(juce::TextButton::textColourOffId, Theme::Colours::text());
    presetButton.setTooltip("Browse presets");
    presetButton.onClick = [this]()
    {
        if (onPresetClicked)
            onPresetClicked(effectIndex);
    };
    addAndMakeVisible(presetButton);

    // Tempo sync controls (for Delay effect only)
    syncButton.setColour(juce::ToggleButton::textColourId, Theme::Colours::textMuted());
    syncButton.setColour(juce::ToggleButton::tickColourId, Theme::Colours::accent());
    syncButton.setTooltip("Sync delay time to tempo");
    syncButton.onClick = [this]()
    {
        if (syncButton.getToggleState() && isDelayEffect)
        {
            // Apply tempo sync based on selected note value
            float noteValue = 1.0f;  // Default: quarter note
            int selected = noteValueCombo.getSelectedId();
            switch (selected)
            {
                case 1: noteValue = 2.0f; break;      // 1/2 (half note)
                case 2: noteValue = 1.0f; break;      // 1/4 (quarter)
                case 3: noteValue = 0.5f; break;      // 1/8 (eighth)
                case 4: noteValue = 0.25f; break;     // 1/16 (sixteenth)
                case 5: noteValue = 1.5f; break;      // dotted 1/4
                case 6: noteValue = 0.75f; break;     // dotted 1/8
                default: noteValue = 1.0f; break;
            }

            if (auto* delayEffect = dynamic_cast<DelayEffect*>(effectSlot))
                delayEffect->syncToTempo(currentTempo, noteValue);
        }
    };
    syncButton.setVisible(false);
    addAndMakeVisible(syncButton);

    noteValueCombo.addItem("1/2", 1);
    noteValueCombo.addItem("1/4", 2);
    noteValueCombo.addItem("1/8", 3);
    noteValueCombo.addItem("1/16", 4);
    noteValueCombo.addItem("1/4.", 5);  // Dotted quarter
    noteValueCombo.addItem("1/8.", 6);  // Dotted eighth
    noteValueCombo.setSelectedId(2);  // Default: 1/4
    noteValueCombo.setTooltip("Note value for tempo sync");
    noteValueCombo.onChange = [this]()
    {
        if (syncButton.getToggleState())
            syncButton.onClick();  // Re-apply sync
    };
    noteValueCombo.setVisible(false);
    addAndMakeVisible(noteValueCombo);
}

EffectSlotComponent::~EffectSlotComponent()
{
    clearParameterControls();
}

void EffectSlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Drop shadow
    DrawingHelpers::drawShadow(g, bounds, Theme::Shadows::sm, Theme::cornerRadius);

    auto innerBounds = bounds.reduced(1);

    // Background gradient
    auto bgGradient = juce::ColourGradient(
        Theme::colour(Theme::bgSlot).brighter(0.05f), innerBounds.getX(), innerBounds.getY(),
        Theme::colour(Theme::bgSlot).darker(0.05f), innerBounds.getX(), innerBounds.getBottom(),
        false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(innerBounds, Theme::cornerRadius);

    // Header area with gradient
    auto headerBounds = innerBounds.removeFromTop(static_cast<float>(headerHeight));
    auto headerGradient = juce::ColourGradient(
        Theme::colour(Theme::bgHover).brighter(0.08f), headerBounds.getX(), headerBounds.getY(),
        Theme::colour(Theme::bgHover).darker(0.05f), headerBounds.getX(), headerBounds.getBottom(),
        false);
    g.setGradientFill(headerGradient);
    g.fillRoundedRectangle(headerBounds.withTrimmedBottom(-4), Theme::cornerRadius);

    // Top highlight
    DrawingHelpers::drawTopHighlight(g, bounds.reduced(1), Theme::cornerRadius, 0.08f);

    // Bypass glow when effect is active (not bypassed)
    bool isBypassed = effectSlot ? effectSlot->isBypassed() : false;
    if (effectSlot && !isBypassed)
    {
        // Active effect glow
        DrawingHelpers::drawGlow(g, bounds.reduced(1), Theme::Colours::accent(),
                                  Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle * 0.6f);
    }
    else if (isBypassed)
    {
        // Bypassed indicator - dim orange glow
        DrawingHelpers::drawGlow(g, bounds.reduced(1), Theme::Colours::warning(),
                                  Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle * 0.3f);
    }

    // Border
    g.setColour(effectSlot && !isBypassed
                ? Theme::Colours::accent().withAlpha(0.4f)
                : Theme::colour(Theme::border));
    g.drawRoundedRectangle(bounds.reduced(1), Theme::cornerRadius, 1.0f);

    // Drag handle (≡) on the left of header
    auto handleArea = getLocalBounds().removeFromTop(headerHeight).removeFromLeft(dragHandleWidth + padding);
    handleArea = handleArea.reduced(padding + 2, (headerHeight - 12) / 2);

    g.setColour(Theme::Colours::textMuted());
    int lineSpacing = 4;
    for (int i = 0; i < 3; ++i)
    {
        int y = handleArea.getY() + i * lineSpacing;
        g.drawHorizontalLine(y, static_cast<float>(handleArea.getX()),
                            static_cast<float>(handleArea.getRight()));
    }
}

void EffectSlotComponent::resized()
{
    auto bounds = getLocalBounds().reduced(padding);

    // Header area
    auto header = bounds.removeFromTop(headerHeight - padding * 2);

    // Leave space for drag handle on left
    header.removeFromLeft(dragHandleWidth);

    removeButton.setBounds(header.removeFromRight(28));
    header.removeFromRight(4);

    // Edit button (only for VST3)
    if (isVST3Effect)
    {
        editButton.setBounds(header.removeFromRight(40));
        header.removeFromRight(4);
    }

    bypassButton.setBounds(header.removeFromRight(40));
    header.removeFromRight(4);
    presetButton.setBounds(header.removeFromRight(24));
    header.removeFromRight(4);

    // Tempo sync controls (only for Delay effect)
    if (isDelayEffect)
    {
        noteValueCombo.setBounds(header.removeFromRight(50));
        header.removeFromRight(2);
        syncButton.setBounds(header.removeFromRight(50));
        header.removeFromRight(8);
    }

    nameLabel.setBounds(header);

    bounds.removeFromTop(padding);

    // Parameter controls - grid layout (2 columns)
    if (!paramControls.empty())
    {
        int numParams = static_cast<int>(paramControls.size());
        int cols = 2;
        int rows = (numParams + cols - 1) / cols;
        int colWidth = bounds.getWidth() / cols;

        for (int i = 0; i < numParams; ++i)
        {
            int row = i / cols;
            int col = i % cols;

            auto& ctrl = paramControls[static_cast<size_t>(i)];
            auto area = juce::Rectangle<int>(
                bounds.getX() + col * colWidth,
                bounds.getY() + row * paramRowHeight,
                colWidth - padding,
                paramRowHeight - padding
            );

            auto labelArea = area.removeFromTop(16);
            ctrl.label->setBounds(labelArea);

            area.removeFromTop(2);
            ctrl.slider->setBounds(area);
        }
    }
}

void EffectSlotComponent::setEffect(EffectSlot* slot, int index)
{
    effectSlot = slot;
    effectIndex = index;

    // Check if this is a VST3 plugin
    isVST3Effect = (dynamic_cast<VST3EffectSlot*>(slot) != nullptr);
    editButton.setVisible(isVST3Effect);

    // Check if this is a Delay effect (for tempo sync)
    isDelayEffect = (dynamic_cast<DelayEffect*>(slot) != nullptr);
    syncButton.setVisible(isDelayEffect);
    noteValueCombo.setVisible(isDelayEffect);

    if (slot)
    {
        nameLabel.setText(slot->getName(), juce::dontSendNotification);
        bypassButton.setToggleState(slot->isBypassed(), juce::dontSendNotification);
        buildParameterControls();
    }
    else
    {
        nameLabel.setText("No Effect", juce::dontSendNotification);
        clearParameterControls();
    }

    resized();
}

int EffectSlotComponent::getPreferredHeight() const
{
    if (!effectSlot || effectSlot->getNumParameters() == 0)
        return headerHeight + padding * 2;

    int numParams = effectSlot->getNumParameters();
    int rows = (numParams + 1) / 2; // 2 columns
    return headerHeight + (rows * paramRowHeight) + padding * 2;
}

void EffectSlotComponent::buildParameterControls()
{
    clearParameterControls();

    if (!effectSlot)
        return;

    int numParams = effectSlot->getNumParameters();

    for (int i = 0; i < numParams; ++i)
    {
        ParamControl ctrl;

        ctrl.label = std::make_unique<juce::Label>();
        ctrl.label->setText(effectSlot->getParameterName(i), juce::dontSendNotification);
        ctrl.label->setColour(juce::Label::textColourId, Theme::Colours::textMuted());
        ctrl.label->setFont(juce::Font(11.0f));
        ctrl.label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(ctrl.label.get());

        ctrl.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                      juce::Slider::TextBoxBelow);
        ctrl.slider->setRange(static_cast<double>(effectSlot->getParameterMin(i)),
                              static_cast<double>(effectSlot->getParameterMax(i)),
                              0.01);
        ctrl.slider->setValue(static_cast<double>(effectSlot->getParameter(i)),
                              juce::dontSendNotification);
        ctrl.slider->setColour(juce::Slider::rotarySliderFillColourId,
                               Theme::Colours::accent());
        ctrl.slider->setColour(juce::Slider::thumbColourId,
                               Theme::Colours::accent());
        ctrl.slider->setColour(juce::Slider::textBoxTextColourId,
                               Theme::Colours::text());
        ctrl.slider->setColour(juce::Slider::textBoxBackgroundColourId,
                               juce::Colour(0x00000000));
        ctrl.slider->setColour(juce::Slider::textBoxOutlineColourId,
                               juce::Colour(0x00000000));
        ctrl.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 14);

        int paramIndex = i;
        ctrl.slider->onValueChange = [this, paramIndex]()
        {
            if (effectSlot && paramIndex < static_cast<int>(paramControls.size()))
            {
                float value = static_cast<float>(paramControls[static_cast<size_t>(paramIndex)].slider->getValue());
                effectSlot->setParameter(paramIndex, value);
                if (onParameterChanged)
                    onParameterChanged(effectIndex, paramIndex, value);
            }
        };

        // Fine control mode for precision (Shift+drag)
        ctrl.slider->setVelocityBasedMode(true);
        ctrl.slider->setVelocityModeParameters(0.5, 1, 0.1, false);

        addAndMakeVisible(ctrl.slider.get());
        paramControls.push_back(std::move(ctrl));
    }
}

void EffectSlotComponent::clearParameterControls()
{
    for (auto& ctrl : paramControls)
    {
        if (ctrl.slider)
            removeChildComponent(ctrl.slider.get());
        if (ctrl.label)
            removeChildComponent(ctrl.label.get());
    }
    paramControls.clear();
}

void EffectSlotComponent::mouseDown(const juce::MouseEvent& e)
{
    // Only start drag from header area (drag handle region)
    auto headerArea = getLocalBounds().removeFromTop(headerHeight);
    auto dragHandleArea = headerArea.removeFromLeft(dragHandleWidth + padding);

    if (dragHandleArea.contains(e.getPosition()))
    {
        dragging = true;
        dragStartPos = e.getPosition();
        dragStartY = getY();
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    }
}

void EffectSlotComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragging)
        return;

    int deltaY = e.getPosition().y - dragStartPos.y;
    int newY = dragStartY + deltaY;

    // Calculate which slot we're dragging over
    int slotHeight = getPreferredHeight();
    int targetIndex = (newY + slotHeight / 2) / slotHeight;

    if (targetIndex != effectIndex && targetIndex >= 0 && onReorder)
    {
        onReorder(effectIndex, targetIndex);
    }
}

void EffectSlotComponent::mouseUp(const juce::MouseEvent& /*e*/)
{
    if (dragging)
    {
        dragging = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void EffectSlotComponent::setTempo(double bpm)
{
    currentTempo = bpm;

    // Re-apply sync if enabled
    if (isDelayEffect && syncButton.getToggleState())
        syncButton.onClick();
}
