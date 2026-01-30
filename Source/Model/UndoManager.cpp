#include "UndoManager.h"

UndoManager::UndoManager(size_t maxHistorySize)
    : maxSize(maxHistorySize)
{
}

void UndoManager::execute(std::unique_ptr<UndoableCommand> command)
{
    if (!command)
        return;

    // Try to merge with previous command (e.g., continuous drags)
    if (!undoStack.empty())
    {
        auto* lastCmd = undoStack.back().get();
        if (lastCmd->canMergeWith(command.get()))
        {
            lastCmd->mergeWith(command.get());
            // Re-execute merged command
            lastCmd->execute();
            notifyStateChanged();
            return;
        }
    }

    // Execute the new command
    if (command->execute())
    {
        undoStack.push_back(std::move(command));

        // Clear redo stack (new action invalidates redo history)
        redoStack.clear();

        // Trim history if exceeds max size
        trimHistory();

        notifyStateChanged();
    }
}

bool UndoManager::undo()
{
    if (undoStack.empty())
        return false;

    auto command = std::move(undoStack.back());
    undoStack.pop_back();

    if (command->undo())
    {
        redoStack.push_back(std::move(command));
        notifyStateChanged();
        return true;
    }

    return false;
}

bool UndoManager::redo()
{
    if (redoStack.empty())
        return false;

    auto command = std::move(redoStack.back());
    redoStack.pop_back();

    if (command->execute())
    {
        undoStack.push_back(std::move(command));
        notifyStateChanged();
        return true;
    }

    return false;
}

void UndoManager::clear()
{
    undoStack.clear();
    redoStack.clear();
    notifyStateChanged();
}

juce::String UndoManager::getUndoDescription() const
{
    if (undoStack.empty())
        return "Undo";

    return "Undo " + undoStack.back()->getDescription();
}

juce::String UndoManager::getRedoDescription() const
{
    if (redoStack.empty())
        return "Redo";

    return "Redo " + redoStack.back()->getDescription();
}

void UndoManager::trimHistory()
{
    while (undoStack.size() > maxSize)
        undoStack.erase(undoStack.begin());
}

void UndoManager::notifyStateChanged()
{
    if (onStateChanged)
        onStateChanged();
}
