#include "ModernLookAndFeel.h"
#include "DrawingHelpers.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    // Set default colours
    setColour(juce::ResizableWindow::backgroundColourId, Theme::Colours::background());
    setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgSlot));
    setColour(juce::TextButton::textColourOffId, Theme::Colours::text());
    setColour(juce::TextButton::textColourOnId, Theme::colour(Theme::textOnAccent));
    setColour(juce::ComboBox::backgroundColourId, Theme::colour(Theme::bgSlot));
    setColour(juce::ComboBox::textColourId, Theme::Colours::text());
    setColour(juce::ComboBox::outlineColourId, Theme::colour(Theme::border));
    setColour(juce::ComboBox::arrowColourId, Theme::Colours::textMuted());
    setColour(juce::PopupMenu::backgroundColourId, Theme::colour(Theme::bgPanel));
    setColour(juce::PopupMenu::textColourId, Theme::Colours::text());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Theme::Colours::accent());
    setColour(juce::PopupMenu::highlightedTextColourId, Theme::colour(Theme::textOnAccent));
    setColour(juce::Label::textColourId, Theme::Colours::text());
    setColour(juce::Slider::rotarySliderFillColourId, Theme::Colours::accent());
    setColour(juce::Slider::thumbColourId, Theme::Colours::accent());
    setColour(juce::Slider::trackColourId, Theme::colour(Theme::bgHover));
    setColour(juce::Slider::textBoxTextColourId, Theme::Colours::text());
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ScrollBar::thumbColourId, Theme::colour(Theme::bgHover));
    setColour(juce::TextEditor::backgroundColourId, Theme::colour(Theme::bgSlot));
    setColour(juce::TextEditor::textColourId, Theme::Colours::text());
    setColour(juce::TextEditor::outlineColourId, Theme::colour(Theme::border));
    setColour(juce::TextEditor::focusedOutlineColourId, Theme::Colours::accent());
    setColour(juce::CaretComponent::caretColourId, Theme::Colours::accent());
    setColour(juce::TooltipWindow::backgroundColourId, Theme::colour(Theme::bgPanel));
    setColour(juce::TooltipWindow::textColourId, Theme::Colours::text());
    setColour(juce::TooltipWindow::outlineColourId, Theme::colour(Theme::border));
}

//=============================================================================
// BUTTONS
//=============================================================================
void ModernLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                              juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    // Drop shadow (only when not pressed)
    if (!shouldDrawButtonAsDown)
    {
        DrawingHelpers::drawShadow(g, bounds, Theme::Shadows::sm, Theme::cornerRadiusSm);
    }

    // Determine button color based on state
    auto baseColour = backgroundColour;
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.1f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.08f);

    // Gradient fill
    auto gradient = shouldDrawButtonAsDown
        ? Theme::Gradients::buttonPressed(bounds, baseColour)
        : Theme::Gradients::buttonNormal(bounds, baseColour);

    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, Theme::cornerRadiusSm);

    // Top highlight (beveled effect) - only when not pressed
    if (!shouldDrawButtonAsDown)
    {
        DrawingHelpers::drawTopHighlight(g, bounds, Theme::cornerRadiusSm, 0.12f);
    }

    // Border
    g.setColour(Theme::colour(Theme::border));
    g.drawRoundedRectangle(bounds, Theme::cornerRadiusSm, 1.0f);

    // Glow on hover
    if (shouldDrawButtonAsHighlighted && !shouldDrawButtonAsDown && button.isEnabled())
    {
        auto glowColour = button.getToggleState()
            ? Theme::Colours::warning()
            : Theme::Colours::accent();
        DrawingHelpers::drawGlow(g, bounds, glowColour,
                                  Theme::Glow::radiusSmall, Theme::Glow::intensitySubtle);
    }
}

void ModernLookAndFeel::drawButtonText(juce::Graphics& g,
                                        juce::TextButton& button,
                                        bool /*shouldDrawButtonAsHighlighted*/,
                                        bool /*shouldDrawButtonAsDown*/)
{
    auto textColour = button.getToggleState()
        ? button.findColour(juce::TextButton::textColourOnId)
        : button.findColour(juce::TextButton::textColourOffId);

    g.setColour(textColour);

    auto bounds = button.getLocalBounds().toFloat();
    auto text = button.getButtonText();

    // Check for transport button icons and draw as paths for reliability
    if (text == juce::CharPointer_UTF8("\xe2\x96\xb6"))  // Play ▶
    {
        auto iconSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;
        auto centre = bounds.getCentre();

        juce::Path playIcon;
        playIcon.addTriangle(
            centre.x - iconSize * 0.4f, centre.y - iconSize * 0.5f,
            centre.x - iconSize * 0.4f, centre.y + iconSize * 0.5f,
            centre.x + iconSize * 0.5f, centre.y
        );
        g.fillPath(playIcon);
    }
    else if (text == juce::CharPointer_UTF8("\xe2\x8f\xb8"))  // Pause ⏸
    {
        auto iconSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
        auto centre = bounds.getCentre();
        auto barWidth = iconSize * 0.3f;
        auto barHeight = iconSize;
        auto gap = iconSize * 0.25f;

        g.fillRoundedRectangle(centre.x - gap - barWidth, centre.y - barHeight * 0.5f,
                                barWidth, barHeight, 2.0f);
        g.fillRoundedRectangle(centre.x + gap, centre.y - barHeight * 0.5f,
                                barWidth, barHeight, 2.0f);
    }
    else if (text == juce::CharPointer_UTF8("\xe2\x8f\xb9"))  // Stop ⏹
    {
        auto iconSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
        auto centre = bounds.getCentre();

        g.fillRoundedRectangle(centre.x - iconSize * 0.5f, centre.y - iconSize * 0.5f,
                                iconSize, iconSize, 2.0f);
    }
    else
    {
        // Default text rendering for regular buttons
        auto font = juce::Font(13.0f, juce::Font::bold);
        g.setFont(font);
        g.drawText(text, button.getLocalBounds(), juce::Justification::centred, true);
    }
}

//=============================================================================
// SLIDERS
//=============================================================================
void ModernLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Background arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius, radius,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(Theme::colour(Theme::bgHover));
    g.strokePath(backgroundArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius, radius,
                            0.0f, rotaryStartAngle, angle, true);

    auto fillColour = slider.findColour(juce::Slider::rotarySliderFillColourId);

    // Multi-layer glow for value arc (FL Studio style)
    for (float i = 3.0f; i > 0; i -= 1.0f)
    {
        g.setColour(fillColour.withAlpha(0.12f * (1.0f - i / 3.0f)));
        g.strokePath(valueArc, juce::PathStrokeType(4.0f + i * 3.0f,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Crisp value arc
    g.setColour(fillColour);
    g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Knob center with gradient (3D effect)
    auto knobRadius = radius * 0.4f;
    auto knobBounds = juce::Rectangle<float>(centreX - knobRadius, centreY - knobRadius,
                                              knobRadius * 2, knobRadius * 2);

    // Knob gradient (top-lit)
    auto knobGradient = juce::ColourGradient(
        Theme::colour(Theme::bgHover).brighter(0.15f), knobBounds.getX(), knobBounds.getY(),
        Theme::colour(Theme::bgHover).darker(0.1f), knobBounds.getX(), knobBounds.getBottom(),
        false);

    g.setGradientFill(knobGradient);
    g.fillEllipse(knobBounds);

    // Knob highlight arc (top half only)
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.fillEllipse(knobBounds.reduced(knobRadius * 0.15f).withBottom(centreY));

    // Knob border
    g.setColour(Theme::colour(Theme::border));
    g.drawEllipse(knobBounds, 1.0f);

    // Pointer
    juce::Path pointer;
    auto pointerLength = radius * 0.55f;
    auto pointerThickness = 3.0f;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -radius + 6.0f,
                                 pointerThickness, pointerLength, 2.0f);
    g.setColour(Theme::Colours::text());
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
}

void ModernLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                          juce::Slider::SliderStyle style,
                                          juce::Slider& slider)
{
    bool isVertical = (style == juce::Slider::LinearVertical ||
                       style == juce::Slider::LinearBarVertical);

    auto trackWidth = 4.0f;
    juce::Rectangle<float> track;

    if (isVertical)
    {
        track = juce::Rectangle<float>(
            static_cast<float>(x) + static_cast<float>(width) * 0.5f - trackWidth * 0.5f,
            static_cast<float>(y),
            trackWidth,
            static_cast<float>(height)
        );
    }
    else
    {
        track = juce::Rectangle<float>(
            static_cast<float>(x),
            static_cast<float>(y) + static_cast<float>(height) * 0.5f - trackWidth * 0.5f,
            static_cast<float>(width),
            trackWidth
        );
    }

    // Background track
    g.setColour(Theme::colour(Theme::bgHover));
    g.fillRoundedRectangle(track, 2.0f);

    // Value track
    auto fillColour = slider.findColour(juce::Slider::trackColourId);
    if (fillColour == Theme::colour(Theme::bgHover))
        fillColour = Theme::Colours::accent();

    juce::Rectangle<float> filledTrack;
    if (isVertical)
    {
        filledTrack = track.withTop(sliderPos);
    }
    else
    {
        filledTrack = track.withRight(sliderPos);
    }

    g.setColour(fillColour);
    g.fillRoundedRectangle(filledTrack, 2.0f);

    // Thumb
    auto thumbSize = 14.0f;
    juce::Rectangle<float> thumbBounds;

    if (isVertical)
    {
        thumbBounds = juce::Rectangle<float>(
            static_cast<float>(x) + static_cast<float>(width) * 0.5f - thumbSize * 0.5f,
            sliderPos - thumbSize * 0.5f,
            thumbSize, thumbSize
        );
    }
    else
    {
        thumbBounds = juce::Rectangle<float>(
            sliderPos - thumbSize * 0.5f,
            static_cast<float>(y) + static_cast<float>(height) * 0.5f - thumbSize * 0.5f,
            thumbSize, thumbSize
        );
    }

    // Thumb glow
    g.setColour(fillColour.withAlpha(0.3f));
    g.fillEllipse(thumbBounds.expanded(3.0f));

    // Thumb fill
    g.setColour(fillColour);
    g.fillEllipse(thumbBounds);

    // Thumb center
    g.setColour(Theme::Colours::text());
    g.fillEllipse(thumbBounds.reduced(4.0f));
}

//=============================================================================
// COMBOBOX
//=============================================================================
void ModernLookAndFeel::drawComboBox(juce::Graphics& g,
                                      int width, int height,
                                      bool isButtonDown,
                                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                      juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
    auto cornerRadius = Theme::cornerRadiusSm;

    // Drop shadow
    if (!isButtonDown)
    {
        DrawingHelpers::drawShadow(g, bounds, Theme::Shadows::sm, cornerRadius);
    }

    // Background with gradient
    auto bgColour = box.findColour(juce::ComboBox::backgroundColourId);
    if (isButtonDown)
        bgColour = bgColour.darker(0.05f);

    auto gradient = isButtonDown
        ? Theme::Gradients::buttonPressed(bounds, bgColour)
        : Theme::Gradients::buttonNormal(bounds, bgColour);

    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // Top highlight
    if (!isButtonDown)
    {
        DrawingHelpers::drawTopHighlight(g, bounds, cornerRadius, 0.08f);
    }

    // Border
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

    // Arrow with slight glow when hovered
    auto arrowZone = juce::Rectangle<int>(width - 24, 0, 20, height);
    juce::Path arrow;
    arrow.addTriangle(
        static_cast<float>(arrowZone.getX()) + 3.0f, static_cast<float>(arrowZone.getCentreY()) - 2.0f,
        static_cast<float>(arrowZone.getRight()) - 3.0f, static_cast<float>(arrowZone.getCentreY()) - 2.0f,
        static_cast<float>(arrowZone.getCentreX()), static_cast<float>(arrowZone.getCentreY()) + 4.0f
    );

    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.fillPath(arrow);
}

void ModernLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

    // Drop shadow for popup
    DrawingHelpers::drawShadow(g, bounds.reduced(4), Theme::Shadows::lg, Theme::cornerRadius);

    // Background with gradient
    auto gradient = Theme::Gradients::panel(bounds);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, Theme::cornerRadius);

    // Border
    g.setColour(Theme::colour(Theme::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), Theme::cornerRadius, 1.0f);
}

void ModernLookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                           const juce::Rectangle<int>& area,
                                           bool isSeparator,
                                           bool /*isActive*/,
                                           bool isHighlighted,
                                           bool isTicked,
                                           bool /*hasSubMenu*/,
                                           const juce::String& text,
                                           const juce::String& /*shortcutKeyText*/,
                                           const juce::Drawable* /*icon*/,
                                           const juce::Colour* textColourToUse)
{
    if (isSeparator)
    {
        auto r = area.reduced(5, 0).toFloat();
        r.removeFromTop(static_cast<float>(area.getHeight()) / 2.0f - 0.5f);
        g.setColour(Theme::colour(Theme::divider));
        g.fillRect(r.removeFromTop(1.0f));
        return;
    }

    auto bounds = area.reduced(2);

    if (isHighlighted)
    {
        g.setColour(Theme::Colours::accent());
        g.fillRoundedRectangle(bounds.toFloat(), Theme::cornerRadiusSm);
    }

    auto textColour = (textColourToUse != nullptr) ? *textColourToUse
        : (isHighlighted ? Theme::colour(Theme::textOnAccent) : Theme::Colours::text());

    g.setColour(textColour);
    g.setFont(juce::Font(14.0f));

    auto textBounds = bounds.reduced(8, 0);

    if (isTicked)
    {
        auto tickBounds = textBounds.removeFromLeft(20);
        g.drawText(juce::String::charToString(0x2713), tickBounds,
                   juce::Justification::centred, false);
    }

    g.drawText(text, textBounds, juce::Justification::centredLeft, true);
}

//=============================================================================
// LABELS
//=============================================================================
void ModernLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        auto textColour = label.findColour(juce::Label::textColourId);
        g.setColour(textColour);
        g.setFont(label.getFont());

        auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
        g.drawText(label.getText(), textArea, label.getJustificationType(), true);
    }
}

//=============================================================================
// SCROLLBARS
//=============================================================================
void ModernLookAndFeel::drawScrollbar(juce::Graphics& g,
                                       juce::ScrollBar& /*scrollbar*/,
                                       int x, int y, int width, int height,
                                       bool isScrollbarVertical,
                                       int thumbStartPosition, int thumbSize,
                                       bool isMouseOver, bool isMouseDown)
{
    auto thumbColour = Theme::colour(Theme::bgHover);
    if (isMouseDown)
        thumbColour = Theme::Colours::accent();
    else if (isMouseOver)
        thumbColour = thumbColour.brighter(0.2f);

    juce::Rectangle<int> thumbBounds;

    if (isScrollbarVertical)
    {
        thumbBounds = juce::Rectangle<int>(x + 2, thumbStartPosition, width - 4, thumbSize);
    }
    else
    {
        thumbBounds = juce::Rectangle<int>(thumbStartPosition, y + 2, thumbSize, height - 4);
    }

    g.setColour(thumbColour);
    g.fillRoundedRectangle(thumbBounds.toFloat(), 3.0f);
}

//=============================================================================
// TEXT EDITOR
//=============================================================================
void ModernLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                                  juce::TextEditor& textEditor)
{
    g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width),
                           static_cast<float>(height), Theme::cornerRadiusSm);
}

void ModernLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
                                               juce::TextEditor& textEditor)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                          static_cast<float>(height)).reduced(0.5f);

    auto outlineColour = textEditor.hasKeyboardFocus(true)
        ? textEditor.findColour(juce::TextEditor::focusedOutlineColourId)
        : textEditor.findColour(juce::TextEditor::outlineColourId);

    g.setColour(outlineColour);
    g.drawRoundedRectangle(bounds, Theme::cornerRadiusSm, 1.0f);
}

//=============================================================================
// TOOLTIP
//=============================================================================
void ModernLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text,
                                     int width, int height)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

    g.setColour(Theme::colour(Theme::bgPanel));
    g.fillRoundedRectangle(bounds, Theme::cornerRadiusSm);

    g.setColour(Theme::colour(Theme::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), Theme::cornerRadiusSm, 1.0f);

    g.setColour(Theme::Colours::text());
    g.setFont(juce::Font(13.0f));
    g.drawText(text, bounds.reduced(6, 4), juce::Justification::centred, true);
}

