#pragma once

#include <juce_core/juce_core.h>

class UndoableCommand
{
public:
    virtual ~UndoableCommand() = default;

    // Execute the command (called on first execute and redo)
    virtual bool execute() = 0;

    // Undo the command
    virtual bool undo() = 0;

    // Description for UI display
    virtual juce::String getDescription() const = 0;

    // Merge continuous operations (e.g., drag gestures)
    virtual bool canMergeWith(const UndoableCommand* /*other*/) const { return false; }
    virtual void mergeWith(const UndoableCommand* /*other*/) {}

    // Timestamp for merge window (500ms for continuous drags)
    juce::int64 timestamp = juce::Time::currentTimeMillis();

protected:
    static constexpr juce::int64 mergeWindowMs = 500;
};
