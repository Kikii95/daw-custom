#include "EffectSlotComponent.h"

namespace Colours
{
    constexpr juce::uint32 background = 0xff2a2a2a;
    constexpr juce::uint32 headerBg = 0xff333333;
    constexpr juce::uint32 bypassOff = 0xff424242;
    constexpr juce::uint32 bypassOn = 0xffff9800;
    constexpr juce::uint32 removeBtn = 0xfff44336;
    constexpr juce::uint32 knobAccent = 0xff64b5f6;
    constexpr juce::uint32 textPrimary = 0xffffffff;
    constexpr juce::uint32 textSecondary = 0xffaaaaaa;
}

EffectSlotComponent::EffectSlotComponent()
{
    nameLabel.setColour(juce::Label::textColourId, juce::Colour(Colours::textPrimary));
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(Colours::bypassOff));
    bypassButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(Colours::bypassOn));
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colour(Colours::textPrimary));
    bypassButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff000000));
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

    removeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(Colours::removeBtn));
    removeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(Colours::textPrimary));
    removeButton.onClick = [this]()
    {
        if (onRemoveClicked)
            onRemoveClicked(effectIndex);
    };
    addAndMakeVisible(removeButton);
}

EffectSlotComponent::~EffectSlotComponent()
{
    clearParameterControls();
}

void EffectSlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(juce::Colour(Colours::background));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Header background
    auto headerBounds = bounds.removeFromTop(headerHeight);
    g.setColour(juce::Colour(Colours::headerBg));
    g.fillRoundedRectangle(headerBounds.toFloat().withTrimmedBottom(2), 4.0f);

    // Outline
    g.setColour(juce::Colour(0xff444444));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

void EffectSlotComponent::resized()
{
    auto bounds = getLocalBounds().reduced(padding);

    // Header area
    auto header = bounds.removeFromTop(headerHeight - padding * 2);
    removeButton.setBounds(header.removeFromRight(28));
    header.removeFromRight(4);
    bypassButton.setBounds(header.removeFromRight(40));
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
        ctrl.label->setColour(juce::Label::textColourId, juce::Colour(Colours::textSecondary));
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
                               juce::Colour(Colours::knobAccent));
        ctrl.slider->setColour(juce::Slider::thumbColourId,
                               juce::Colour(Colours::knobAccent));
        ctrl.slider->setColour(juce::Slider::textBoxTextColourId,
                               juce::Colour(Colours::textPrimary));
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
