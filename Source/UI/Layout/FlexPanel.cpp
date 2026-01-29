#include "FlexPanel.h"

FlexPanel::FlexPanel(Orientation orient)
    : orientation(orient)
{
}

void FlexPanel::paint(juce::Graphics& g)
{
    g.fillAll(Theme::Colours::background());

    // Draw dividers
    for (size_t i = 0; i < panels.size() - 1; ++i)
    {
        auto dividerBounds = getDividerBounds(static_cast<int>(i));
        bool isHovered = (getDividerIndex(getMouseXYRelative().x, getMouseXYRelative().y) == static_cast<int>(i));
        bool isDragging = (draggingDivider == static_cast<int>(i));

        if (isDragging)
            g.setColour(Theme::Colours::accent());
        else if (isHovered)
            g.setColour(Theme::colour(Theme::bgHover));
        else
            g.setColour(Theme::colour(Theme::divider));

        g.fillRect(dividerBounds);

        // Draw grip lines
        g.setColour(Theme::colour(Theme::border));
        if (orientation == Orientation::Horizontal)
        {
            int cx = dividerBounds.getCentreX();
            for (int dy = -8; dy <= 8; dy += 4)
            {
                g.fillRect(cx - 1, dividerBounds.getCentreY() + dy, 2, 2);
            }
        }
        else
        {
            int cy = dividerBounds.getCentreY();
            for (int dx = -8; dx <= 8; dx += 4)
            {
                g.fillRect(dividerBounds.getCentreX() + dx, cy - 1, 2, 2);
            }
        }
    }
}

void FlexPanel::resized()
{
    if (panels.empty())
        return;

    normalizeSizes();

    auto bounds = getLocalBounds();
    int totalDividerSpace = dividerSize * (static_cast<int>(panels.size()) - 1);
    int availableSpace = (orientation == Orientation::Horizontal)
        ? bounds.getWidth() - totalDividerSpace
        : bounds.getHeight() - totalDividerSpace;

    int pos = 0;
    for (size_t i = 0; i < panels.size(); ++i)
    {
        int panelSize = static_cast<int>(availableSpace * panelSizes[i]);

        // Ensure minimum size
        panelSize = std::max(panelSize, minPanelSize);

        juce::Rectangle<int> panelBounds;
        if (orientation == Orientation::Horizontal)
        {
            panelBounds = juce::Rectangle<int>(pos, 0, panelSize, bounds.getHeight());
        }
        else
        {
            panelBounds = juce::Rectangle<int>(0, pos, bounds.getWidth(), panelSize);
        }

        if (panels[i].component)
        {
            panels[i].component->setBounds(panelBounds);
        }

        pos += panelSize + dividerSize;
    }
}

void FlexPanel::mouseDown(const juce::MouseEvent& e)
{
    draggingDivider = getDividerIndex(e.x, e.y);

    if (draggingDivider >= 0 && draggingDivider < static_cast<int>(panels.size()) - 1)
    {
        dragStartPos = (orientation == Orientation::Horizontal) ? e.x : e.y;
        dragStartSize1 = panelSizes[static_cast<size_t>(draggingDivider)];
        dragStartSize2 = panelSizes[static_cast<size_t>(draggingDivider) + 1];
    }
}

void FlexPanel::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingDivider < 0 || draggingDivider >= static_cast<int>(panels.size()) - 1)
        return;

    int currentPos = (orientation == Orientation::Horizontal) ? e.x : e.y;
    int delta = currentPos - dragStartPos;

    int totalDividerSpace = dividerSize * (static_cast<int>(panels.size()) - 1);
    int availableSpace = ((orientation == Orientation::Horizontal)
        ? getWidth() : getHeight()) - totalDividerSpace;

    float deltaRatio = static_cast<float>(delta) / static_cast<float>(availableSpace);

    float newSize1 = dragStartSize1 + deltaRatio;
    float newSize2 = dragStartSize2 - deltaRatio;

    // Enforce minimum sizes
    float minRatio = static_cast<float>(minPanelSize) / static_cast<float>(availableSpace);
    newSize1 = std::max(newSize1, minRatio);
    newSize2 = std::max(newSize2, minRatio);

    // Adjust if constraints are violated
    if (newSize1 + newSize2 > dragStartSize1 + dragStartSize2)
    {
        float excess = (newSize1 + newSize2) - (dragStartSize1 + dragStartSize2);
        if (newSize1 > dragStartSize1)
            newSize1 -= excess;
        else
            newSize2 -= excess;
    }

    panelSizes[static_cast<size_t>(draggingDivider)] = newSize1;
    panelSizes[static_cast<size_t>(draggingDivider) + 1] = newSize2;

    resized();
    repaint();
}

void FlexPanel::mouseUp(const juce::MouseEvent& /*e*/)
{
    draggingDivider = -1;
    repaint();
}

void FlexPanel::mouseMove(const juce::MouseEvent& e)
{
    int divider = getDividerIndex(e.x, e.y);

    if (divider >= 0)
    {
        if (orientation == Orientation::Horizontal)
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        else
            setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    }
    else
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    repaint();
}

void FlexPanel::addPanel(juce::Component* component, float initialSize)
{
    PanelInfo info;
    info.component = component;
    info.size = initialSize;

    panels.push_back(info);
    panelSizes.push_back(initialSize);

    if (component)
        addAndMakeVisible(component);

    normalizeSizes();
    resized();
}

void FlexPanel::removePanel(juce::Component* component)
{
    for (size_t i = 0; i < panels.size(); ++i)
    {
        if (panels[i].component == component)
        {
            if (component)
                removeChildComponent(component);

            panels.erase(panels.begin() + static_cast<long>(i));
            panelSizes.erase(panelSizes.begin() + static_cast<long>(i));
            break;
        }
    }

    normalizeSizes();
    resized();
}

void FlexPanel::clearPanels()
{
    for (auto& panel : panels)
    {
        if (panel.component)
            removeChildComponent(panel.component);
    }
    panels.clear();
    panelSizes.clear();
}

void FlexPanel::setOrientation(Orientation newOrientation)
{
    if (orientation != newOrientation)
    {
        orientation = newOrientation;
        resized();
    }
}

void FlexPanel::setPanelSizes(const std::vector<float>& sizes)
{
    if (sizes.size() == panelSizes.size())
    {
        panelSizes = sizes;
        normalizeSizes();
        resized();
    }
}

int FlexPanel::getDividerIndex(int x, int y) const
{
    for (size_t i = 0; i < panels.size() - 1; ++i)
    {
        auto bounds = getDividerBounds(static_cast<int>(i));
        // Expand hit area slightly for easier grabbing
        bounds = bounds.expanded(2);

        if (bounds.contains(x, y))
            return static_cast<int>(i);
    }
    return -1;
}

juce::Rectangle<int> FlexPanel::getDividerBounds(int index) const
{
    if (index < 0 || index >= static_cast<int>(panels.size()) - 1)
        return {};

    auto bounds = getLocalBounds();
    int totalDividerSpace = dividerSize * (static_cast<int>(panels.size()) - 1);
    int availableSpace = (orientation == Orientation::Horizontal)
        ? bounds.getWidth() - totalDividerSpace
        : bounds.getHeight() - totalDividerSpace;

    int pos = 0;
    for (int i = 0; i <= index; ++i)
    {
        pos += static_cast<int>(availableSpace * panelSizes[static_cast<size_t>(i)]);
        if (i < index)
            pos += dividerSize;
    }

    if (orientation == Orientation::Horizontal)
    {
        return juce::Rectangle<int>(pos, 0, dividerSize, bounds.getHeight());
    }
    else
    {
        return juce::Rectangle<int>(0, pos, bounds.getWidth(), dividerSize);
    }
}

void FlexPanel::normalizeSizes()
{
    if (panelSizes.empty())
        return;

    float total = 0.0f;
    for (float size : panelSizes)
        total += size;

    if (total > 0.0f)
    {
        for (float& size : panelSizes)
            size /= total;
    }
    else
    {
        float equalSize = 1.0f / static_cast<float>(panelSizes.size());
        for (float& size : panelSizes)
            size = equalSize;
    }
}
