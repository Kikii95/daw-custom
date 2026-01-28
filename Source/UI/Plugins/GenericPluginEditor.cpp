#include "GenericPluginEditor.h"
#include "../../Audio/Plugins/VST3EffectSlot.h"

GenericPluginEditor::GenericPluginEditor(VST3EffectSlot* effectSlot)
    : slot(effectSlot)
{
    jassert(slot != nullptr);

    buildControls();

    // Calculate size based on parameter count
    int numParams = slot->getNumParameters();
    int rows = (numParams + COLUMNS - 1) / COLUMNS;
    rows = juce::jmax(1, rows);

    preferredWidth = COLUMNS * PARAM_WIDTH + (COLUMNS + 1) * PADDING;
    preferredHeight = rows * PARAM_HEIGHT + (rows + 1) * PADDING + 30; // +30 for header

    setSize(preferredWidth, preferredHeight);

    // Start polling timer
    startTimerHz(10);
}

GenericPluginEditor::~GenericPluginEditor()
{
    stopTimer();
}

void GenericPluginEditor::buildControls()
{
    controls.clear();

    if (!slot)
        return;

    int numParams = slot->getNumParameters();

    for (int i = 0; i < numParams; ++i)
    {
        auto* control = controls.add(new ParameterControl());
        control->paramIndex = i;

        // Create slider
        control->slider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag,
                                                         juce::Slider::TextBoxBelow);
        control->slider->setRange(0.0, 1.0, 0.001);
        control->slider->setValue(slot->getParameter(i), juce::dontSendNotification);
        control->slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);

        // Slider callback
        control->slider->onValueChange = [this, i]()
        {
            if (slot)
            {
                float value = static_cast<float>(controls[i]->slider->getValue());
                slot->setParameter(i, value);
            }
        };

        addAndMakeVisible(*control->slider);

        // Create label
        control->label = std::make_unique<juce::Label>();
        control->label->setText(slot->getParameterName(i), juce::dontSendNotification);
        control->label->setJustificationType(juce::Justification::centred);
        control->label->setFont(juce::Font(11.0f));
        addAndMakeVisible(*control->label);
    }
}

void GenericPluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Header
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRect(0, 0, getWidth(), 28);

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(slot ? slot->getName() : "Plugin",
               10, 0, getWidth() - 20, 28,
               juce::Justification::centredLeft);

    // Border
    g.setColour(juce::Colour(0xff555555));
    g.drawRect(getLocalBounds(), 1);
}

void GenericPluginEditor::resized()
{
    int startY = 30; // After header

    for (int i = 0; i < controls.size(); ++i)
    {
        int col = i % COLUMNS;
        int row = i / COLUMNS;

        int x = PADDING + col * (PARAM_WIDTH + PADDING);
        int y = startY + PADDING + row * (PARAM_HEIGHT + PADDING);

        // Label at top
        controls[i]->label->setBounds(x, y, PARAM_WIDTH, 16);

        // Slider below label
        controls[i]->slider->setBounds(x, y + 18, PARAM_WIDTH, PARAM_HEIGHT - 20);
    }
}

void GenericPluginEditor::timerCallback()
{
    updateControlValues();
}

void GenericPluginEditor::updateControlValues()
{
    if (!slot)
        return;

    for (auto* control : controls)
    {
        float value = slot->getParameter(control->paramIndex);
        if (std::abs(control->slider->getValue() - value) > 0.001)
        {
            control->slider->setValue(value, juce::dontSendNotification);
        }
    }
}
