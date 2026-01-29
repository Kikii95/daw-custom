#include "EffectSlotComponent.h"
#include "UI/Plugins/PluginEditorWindow.h"
#include "UI/Theme/AppTheme.h"
#include "UI/Theme/DrawingHelpers.h"

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
}

void EffectSlotComponent::resized()
{
    auto bounds = getLocalBounds().reduced(padding);

    // Header area
    auto header = bounds.removeFromTop(headerHeight - padding * 2);
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
    header.removeFromRight(8);
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
