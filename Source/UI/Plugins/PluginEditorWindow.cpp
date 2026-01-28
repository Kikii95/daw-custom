#include "PluginEditorWindow.h"
#include "../../Audio/Plugins/VST3EffectSlot.h"

juce::Array<PluginEditorWindow*> PluginEditorWindow::openWindows;

PluginEditorWindow::PluginEditorWindow(VST3EffectSlot* effectSlot)
    : DocumentWindow(effectSlot ? effectSlot->getName() : "Plugin Editor",
                    juce::Colours::darkgrey,
                    DocumentWindow::closeButton | DocumentWindow::minimiseButton),
      slot(effectSlot)
{
    jassert(slot != nullptr);

    setUsingNativeTitleBar(true);
    setResizable(false, false);

    createEditor();

    // Center on screen
    centreWithSize(getWidth(), getHeight());
    setVisible(true);

    // Track this window
    openWindows.add(this);

    DBG("PluginEditorWindow: Opened editor for " + slot->getName());
}

PluginEditorWindow::~PluginEditorWindow()
{
    openWindows.removeFirstMatchingValue(this);

    // Clear content before editor is destroyed
    clearContentComponent();
    editor.reset();
}

void PluginEditorWindow::createEditor()
{
    if (!slot || !slot->hasEditor())
    {
        DBG("PluginEditorWindow: Plugin has no editor");
        return;
    }

    auto* plugin = slot->getPluginInstance();
    if (!plugin)
        return;

    editor.reset(plugin->createEditor());

    if (editor)
    {
        // Set content component
        setContentNonOwned(editor.get(), true);

        // Apply size constraints
        auto* constrainer = editor->getConstrainer();
        if (constrainer)
        {
            setResizeLimits(constrainer->getMinimumWidth(),
                           constrainer->getMinimumHeight(),
                           constrainer->getMaximumWidth(),
                           constrainer->getMaximumHeight());

            // Allow resize if plugin supports it
            if (constrainer->getMinimumWidth() != constrainer->getMaximumWidth() ||
                constrainer->getMinimumHeight() != constrainer->getMaximumHeight())
            {
                setResizable(true, false);
            }
        }
    }
}

void PluginEditorWindow::closeButtonPressed()
{
    // Self-destruct when closed
    delete this;
}

bool PluginEditorWindow::isPluginValid() const
{
    return slot != nullptr && slot->getPluginInstance() != nullptr;
}

PluginEditorWindow* PluginEditorWindow::getOrCreateWindow(VST3EffectSlot* slot)
{
    if (!slot || !slot->hasEditor())
        return nullptr;

    // Check if window already exists
    for (auto* window : openWindows)
    {
        if (window->getEffectSlot() == slot)
        {
            window->toFront(true);
            return window;
        }
    }

    // Create new window
    return new PluginEditorWindow(slot);
}

void PluginEditorWindow::closeWindowFor(VST3EffectSlot* slot)
{
    for (int i = openWindows.size() - 1; i >= 0; --i)
    {
        if (openWindows[i]->getEffectSlot() == slot)
        {
            delete openWindows[i];
            return;
        }
    }
}

void PluginEditorWindow::closeAllWindows()
{
    while (!openWindows.isEmpty())
    {
        delete openWindows.getLast();
    }
}
