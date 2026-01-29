#include "ModernLookAndFeel.h"

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
    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.brighter(0.1f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.05f);

    // Background
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, Theme::cornerRadiusSm);

    // Subtle border
    g.setColour(Theme::colour(Theme::border));
    g.drawRoundedRectangle(bounds, Theme::cornerRadiusSm, 1.0f);

    // Glow on hover/press
    if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
    {
        auto glowColour = button.getToggleState()
            ? Theme::Colours::warning().withAlpha(0.3f)
            : Theme::Colours::accent().withAlpha(0.2f);
        drawGlow(g, bounds, glowColour, 4.0f);
    }
}

void ModernLookAndFeel::drawButtonText(juce::Graphics& g,
                                        juce::TextButton& button,
                                        bool /*shouldDrawButtonAsHighlighted*/,
                                        bool /*shouldDrawButtonAsDown*/)
{
    auto font = juce::Font(13.0f, juce::Font::bold);
    g.setFont(font);

    auto textColour = button.getToggleState()
        ? button.findColour(juce::TextButton::textColourOnId)
        : button.findColour(juce::TextButton::textColourOffId);

    g.setColour(textColour);
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, true);
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
    g.setColour(fillColour);
    g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Glow effect on value arc
    g.setColour(fillColour.withAlpha(0.3f));
    g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Center dot
    auto dotRadius = radius * 0.15f;
    g.setColour(Theme::Colours::text());
    g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2, dotRadius * 2);

    // Pointer
    juce::Path pointer;
    auto pointerLength = radius * 0.6f;
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

    // Background
    auto bgColour = box.findColour(juce::ComboBox::backgroundColourId);
    if (isButtonDown)
        bgColour = bgColour.brighter(0.1f);

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // Border
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

    // Arrow
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

    g.setColour(Theme::colour(Theme::bgPanel));
    g.fillRoundedRectangle(bounds, Theme::cornerRadius);

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

//=============================================================================
// HELPERS
//=============================================================================
void ModernLookAndFeel::drawGlow(juce::Graphics& g, juce::Rectangle<float> bounds,
                                  juce::Colour colour, float radius)
{
    for (float i = radius; i > 0; i -= 1.0f)
    {
        g.setColour(colour.withAlpha(colour.getFloatAlpha() * (1.0f - i / radius) * 0.5f));
        g.drawRoundedRectangle(bounds.expanded(i), Theme::cornerRadiusSm + i, 1.0f);
    }
}
