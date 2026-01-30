#pragma once

#include "UndoableCommand.h"
#include <vector>
#include <memory>
#include <functional>

class UndoManager
{
public:
    explicit UndoManager(size_t maxHistorySize = 100);

    // Execute and record a command
    void execute(std::unique_ptr<UndoableCommand> command);

    // Undo/Redo operations
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    bool undo();
    bool redo();

    // Clear history (for new project)
    void clear();

    // Get descriptions for UI (menu items)
    juce::String getUndoDescription() const;
    juce::String getRedoDescription() const;

    // History info
    size_t getUndoCount() const { return undoStack.size(); }
    size_t getRedoCount() const { return redoStack.size(); }

    // Callback when undo state changes
    std::function<void()> onStateChanged;

private:
    std::vector<std::unique_ptr<UndoableCommand>> undoStack;
    std::vector<std::unique_ptr<UndoableCommand>> redoStack;
    size_t maxSize;

    void trimHistory();
    void notifyStateChanged();
};
