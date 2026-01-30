#pragma once

#include "../UndoableCommand.h"
#include "../Project.h"
#include "../Clip.h"
#include "Audio/AudioMixer.h"
#include "Audio/AudioClip.h"
#include "Audio/FileIO/AudioFileLoader.h"
#include <vector>

// Forward declarations
class AudioMixer;
class AudioFileLoader;

//==============================================================================
// MoveClipCommand - Move one or more clips (horizontal drag)
//==============================================================================
class MoveClipCommand : public UndoableCommand
{
public:
    struct ClipPosition
    {
        juce::Uuid trackId;
        juce::Uuid clipId;
        double startTime;
    };

    MoveClipCommand(Project* project, AudioMixer* mixer,
                    std::vector<ClipPosition> oldPositions,
                    std::vector<ClipPosition> newPositions)
        : project(project)
        , mixer(mixer)
        , oldPositions(std::move(oldPositions))
        , newPositions(std::move(newPositions))
    {
    }

    bool execute() override
    {
        return applyPositions(newPositions);
    }

    bool undo() override
    {
        return applyPositions(oldPositions);
    }

    juce::String getDescription() const override
    {
        if (oldPositions.size() == 1)
            return "Move Clip";
        return "Move " + juce::String(oldPositions.size()) + " Clips";
    }

    bool canMergeWith(const UndoableCommand* other) const override
    {
        auto* otherMove = dynamic_cast<const MoveClipCommand*>(other);
        if (!otherMove)
            return false;

        // Same clips and within time window
        if (otherMove->newPositions.size() != newPositions.size())
            return false;

        for (size_t i = 0; i < newPositions.size(); ++i)
        {
            if (otherMove->newPositions[i].clipId != newPositions[i].clipId)
                return false;
        }

        return (otherMove->timestamp - timestamp) < mergeWindowMs;
    }

    void mergeWith(const UndoableCommand* other) override
    {
        auto* otherMove = dynamic_cast<const MoveClipCommand*>(other);
        if (otherMove)
        {
            newPositions = otherMove->newPositions;
            timestamp = otherMove->timestamp;
        }
    }

private:
    bool applyPositions(const std::vector<ClipPosition>& positions)
    {
        for (const auto& pos : positions)
        {
            if (auto* track = project->getTrack(pos.trackId))
            {
                for (auto& clip : track->clips)
                {
                    if (clip.id == pos.clipId)
                    {
                        clip.startTime = pos.startTime;

                        // Update audio engine
                        if (mixer)
                        {
                            for (int i = 0; i < mixer->getNumTracks(); ++i)
                            {
                                if (auto* at = mixer->getTrack(i))
                                {
                                    if (at->getId() == pos.trackId)
                                    {
                                        if (auto* audioClip = at->getClip(pos.clipId))
                                        {
                                            Clip clipData = audioClip->getClipData();
                                            clipData.startTime = pos.startTime;
                                            audioClip->setClipData(clipData);
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
        return true;
    }

    Project* project;
    AudioMixer* mixer;
    std::vector<ClipPosition> oldPositions;
    std::vector<ClipPosition> newPositions;
};

//==============================================================================
// DeleteClipsCommand - Delete one or more clips
//==============================================================================
class DeleteClipsCommand : public UndoableCommand
{
public:
    struct DeletedClipInfo
    {
        juce::Uuid trackId;
        Clip clipData;
        int trackIndex;
    };

    struct RippleShiftInfo
    {
        juce::Uuid trackId;
        juce::Uuid clipId;
        double shiftAmount;
    };

    DeleteClipsCommand(Project* project, AudioMixer* mixer, AudioFileLoader* fileLoader,
                       std::vector<std::pair<juce::Uuid, juce::Uuid>> clipsToDelete)
        : project(project)
        , mixer(mixer)
        , fileLoader(fileLoader)
        , clipIds(std::move(clipsToDelete))
        , wasRippleMode(project->isRippleEditEnabled())
    {
    }

    bool execute() override
    {
        deletedClips.clear();
        rippleShifts.clear();

        for (const auto& [trackId, clipId] : clipIds)
        {
            if (auto* track = project->getTrack(trackId))
            {
                // Find and store clip data before deletion
                for (const auto& clip : track->clips)
                {
                    if (clip.id == clipId)
                    {
                        int trackIdx = getTrackIndex(trackId);
                        deletedClips.push_back({ trackId, clip, trackIdx });

                        double deletedEnd = clip.getEndTime();
                        double shiftAmount = clip.duration;

                        // Remove from project
                        track->removeClip(clipId);

                        // Remove from audio engine
                        if (mixer && trackIdx >= 0)
                        {
                            if (auto* audioTrack = mixer->getTrack(trackIdx))
                                audioTrack->removeClip(clipId);
                        }

                        // Ripple edit: shift subsequent clips on the same track
                        if (wasRippleMode)
                        {
                            for (auto& nextClip : track->clips)
                            {
                                if (nextClip.startTime >= deletedEnd)
                                {
                                    rippleShifts.push_back({ trackId, nextClip.id, shiftAmount });
                                    nextClip.startTime -= shiftAmount;

                                    // Update audio engine
                                    if (mixer && trackIdx >= 0)
                                    {
                                        if (auto* audioTrack = mixer->getTrack(trackIdx))
                                            audioTrack->updateClipStartTime(nextClip.id, nextClip.startTime);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        return !deletedClips.empty();
    }

    bool undo() override
    {
        // First, reverse ripple shifts (if any)
        for (auto it = rippleShifts.rbegin(); it != rippleShifts.rend(); ++it)
        {
            if (auto* track = project->getTrack(it->trackId))
            {
                if (auto* clip = track->getClip(it->clipId))
                {
                    clip->startTime += it->shiftAmount;

                    // Update audio engine
                    int trackIdx = getTrackIndex(it->trackId);
                    if (mixer && trackIdx >= 0)
                    {
                        if (auto* audioTrack = mixer->getTrack(trackIdx))
                            audioTrack->updateClipStartTime(it->clipId, clip->startTime);
                    }
                }
            }
        }

        // Then restore deleted clips
        for (const auto& info : deletedClips)
        {
            if (auto* track = project->getTrack(info.trackId))
            {
                // Restore clip to project
                track->addClip(info.clipData);

                // Restore to audio engine
                if (mixer && fileLoader && info.trackIndex >= 0)
                {
                    if (auto* audioTrack = mixer->getTrack(info.trackIndex))
                    {
                        if (info.clipData.sourceFile.existsAsFile())
                        {
                            auto result = fileLoader->loadFile(info.clipData.sourceFile);
                            if (result.isValid())
                            {
                                auto audioClip = std::make_unique<AudioClip>();
                                audioClip->loadFromBuffer(std::move(result.buffer),
                                                         result.sampleRate, info.clipData);
                                audioTrack->addClip(std::move(audioClip));
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    juce::String getDescription() const override
    {
        if (clipIds.size() == 1)
            return "Delete Clip";
        return "Delete " + juce::String(clipIds.size()) + " Clips";
    }

private:
    int getTrackIndex(const juce::Uuid& trackId) const
    {
        for (int i = 0; i < project->getNumTracks(); ++i)
        {
            if (auto* t = project->getTrackByIndex(i))
            {
                if (t->id == trackId)
                    return i;
            }
        }
        return -1;
    }

    Project* project;
    AudioMixer* mixer;
    AudioFileLoader* fileLoader;
    std::vector<std::pair<juce::Uuid, juce::Uuid>> clipIds;
    std::vector<DeletedClipInfo> deletedClips;
    std::vector<RippleShiftInfo> rippleShifts;
    bool wasRippleMode = false;
};

//==============================================================================
// AddClipCommand - Add a new clip (for duplicate, paste, import)
//==============================================================================
class AddClipCommand : public UndoableCommand
{
public:
    AddClipCommand(Project* project, AudioMixer* mixer, AudioFileLoader* fileLoader,
                   juce::Uuid targetTrackId, const Clip& clipToAdd)
        : project(project)
        , mixer(mixer)
        , fileLoader(fileLoader)
        , trackId(targetTrackId)
        , clipData(clipToAdd)
    {
    }

    bool execute() override
    {
        if (auto* track = project->getTrack(trackId))
        {
            track->addClip(clipData);

            // Add to audio engine
            int trackIdx = getTrackIndex(trackId);
            if (mixer && fileLoader && trackIdx >= 0)
            {
                if (auto* audioTrack = mixer->getTrack(trackIdx))
                {
                    if (clipData.sourceFile.existsAsFile())
                    {
                        auto result = fileLoader->loadFile(clipData.sourceFile);
                        if (result.isValid())
                        {
                            auto audioClip = std::make_unique<AudioClip>();
                            audioClip->loadFromBuffer(std::move(result.buffer),
                                                     result.sampleRate, clipData);
                            audioTrack->addClip(std::move(audioClip));
                        }
                    }
                }
            }
            return true;
        }
        return false;
    }

    bool undo() override
    {
        if (auto* track = project->getTrack(trackId))
        {
            track->removeClip(clipData.id);

            // Remove from audio engine
            int trackIdx = getTrackIndex(trackId);
            if (mixer && trackIdx >= 0)
            {
                if (auto* audioTrack = mixer->getTrack(trackIdx))
                    audioTrack->removeClip(clipData.id);
            }
            return true;
        }
        return false;
    }

    juce::String getDescription() const override
    {
        return "Add Clip";
    }

    const Clip& getClipData() const { return clipData; }

private:
    int getTrackIndex(const juce::Uuid& tid) const
    {
        for (int i = 0; i < project->getNumTracks(); ++i)
        {
            if (auto* t = project->getTrackByIndex(i))
            {
                if (t->id == tid)
                    return i;
            }
        }
        return -1;
    }

    Project* project;
    AudioMixer* mixer;
    AudioFileLoader* fileLoader;
    juce::Uuid trackId;
    Clip clipData;
};

//==============================================================================
// TrimClipCommand - Trim clip edges
//==============================================================================
class TrimClipCommand : public UndoableCommand
{
public:
    TrimClipCommand(Project* project, AudioMixer* mixer,
                    juce::Uuid trackId, juce::Uuid clipId,
                    double oldStart, double oldDuration, double oldSourceOffset,
                    double newStart, double newDuration, double newSourceOffset)
        : project(project)
        , mixer(mixer)
        , trackId(trackId)
        , clipId(clipId)
        , oldStart(oldStart)
        , oldDuration(oldDuration)
        , oldSourceOffset(oldSourceOffset)
        , newStart(newStart)
        , newDuration(newDuration)
        , newSourceOffset(newSourceOffset)
    {
    }

    bool execute() override
    {
        return applyTrim(newStart, newDuration, newSourceOffset);
    }

    bool undo() override
    {
        return applyTrim(oldStart, oldDuration, oldSourceOffset);
    }

    juce::String getDescription() const override
    {
        return "Trim Clip";
    }

    bool canMergeWith(const UndoableCommand* other) const override
    {
        auto* otherTrim = dynamic_cast<const TrimClipCommand*>(other);
        if (!otherTrim)
            return false;

        return otherTrim->clipId == clipId &&
               (otherTrim->timestamp - timestamp) < mergeWindowMs;
    }

    void mergeWith(const UndoableCommand* other) override
    {
        auto* otherTrim = dynamic_cast<const TrimClipCommand*>(other);
        if (otherTrim)
        {
            newStart = otherTrim->newStart;
            newDuration = otherTrim->newDuration;
            newSourceOffset = otherTrim->newSourceOffset;
            timestamp = otherTrim->timestamp;
        }
    }

private:
    bool applyTrim(double start, double duration, double sourceOffset)
    {
        if (auto* track = project->getTrack(trackId))
        {
            for (auto& clip : track->clips)
            {
                if (clip.id == clipId)
                {
                    clip.startTime = start;
                    clip.duration = duration;
                    clip.sourceStartOffset = sourceOffset;

                    // Update audio engine
                    if (mixer)
                    {
                        for (int i = 0; i < mixer->getNumTracks(); ++i)
                        {
                            if (auto* at = mixer->getTrack(i))
                            {
                                if (at->getId() == trackId)
                                {
                                    at->updateClipTiming(clipId, start, duration, sourceOffset);
                                    break;
                                }
                            }
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }

    Project* project;
    AudioMixer* mixer;
    juce::Uuid trackId;
    juce::Uuid clipId;
    double oldStart, oldDuration, oldSourceOffset;
    double newStart, newDuration, newSourceOffset;
};

//==============================================================================
// MoveClipToTrackCommand - Move clip between tracks
//==============================================================================
class MoveClipToTrackCommand : public UndoableCommand
{
public:
    MoveClipToTrackCommand(Project* project, AudioMixer* mixer, AudioFileLoader* fileLoader,
                           juce::Uuid clipId,
                           juce::Uuid fromTrackId, int fromTrackIdx,
                           juce::Uuid toTrackId, int toTrackIdx)
        : project(project)
        , mixer(mixer)
        , fileLoader(fileLoader)
        , clipId(clipId)
        , fromTrackId(fromTrackId)
        , fromTrackIdx(fromTrackIdx)
        , toTrackId(toTrackId)
        , toTrackIdx(toTrackIdx)
    {
    }

    bool execute() override
    {
        return moveClip(fromTrackId, fromTrackIdx, toTrackId, toTrackIdx);
    }

    bool undo() override
    {
        return moveClip(toTrackId, toTrackIdx, fromTrackId, fromTrackIdx);
    }

    juce::String getDescription() const override
    {
        return "Move Clip to Track";
    }

private:
    bool moveClip(juce::Uuid srcTrackId, int srcIdx, juce::Uuid dstTrackId, int dstIdx)
    {
        auto* srcTrack = project->getTrack(srcTrackId);
        auto* dstTrack = project->getTrack(dstTrackId);

        if (!srcTrack || !dstTrack)
            return false;

        // Find clip in source track
        Clip clipCopy;
        bool found = false;
        for (const auto& c : srcTrack->clips)
        {
            if (c.id == clipId)
            {
                clipCopy = c;
                found = true;
                break;
            }
        }

        if (!found)
            return false;

        // Move in project model
        srcTrack->removeClip(clipId);
        clipCopy.colour = dstTrack->colour;  // Inherit target track colour
        dstTrack->addClip(clipCopy);

        // Move in audio engine
        if (mixer && fileLoader)
        {
            // Remove from source track
            if (srcIdx >= 0 && srcIdx < mixer->getNumTracks())
            {
                if (auto* srcAudioTrack = mixer->getTrack(srcIdx))
                    srcAudioTrack->removeClip(clipId);
            }

            // Add to destination track
            if (dstIdx >= 0 && dstIdx < mixer->getNumTracks())
            {
                if (auto* dstAudioTrack = mixer->getTrack(dstIdx))
                {
                    if (clipCopy.sourceFile.existsAsFile())
                    {
                        auto result = fileLoader->loadFile(clipCopy.sourceFile);
                        if (result.isValid())
                        {
                            auto audioClip = std::make_unique<AudioClip>();
                            audioClip->loadFromBuffer(std::move(result.buffer),
                                                     result.sampleRate, clipCopy);
                            dstAudioTrack->addClip(std::move(audioClip));
                        }
                    }
                }
            }
        }

        return true;
    }

    Project* project;
    AudioMixer* mixer;
    AudioFileLoader* fileLoader;
    juce::Uuid clipId;
    juce::Uuid fromTrackId;
    int fromTrackIdx;
    juce::Uuid toTrackId;
    int toTrackIdx;
};

//==============================================================================
// FadeClipCommand - Change clip fade in/out duration
//==============================================================================
class FadeClipCommand : public UndoableCommand
{
public:
    FadeClipCommand(Project* project, AudioMixer* mixer,
                    juce::Uuid trackId, juce::Uuid clipId,
                    bool isFadeIn,
                    double oldDuration, double newDuration)
        : project(project)
        , mixer(mixer)
        , trackId(trackId)
        , clipId(clipId)
        , isFadeIn(isFadeIn)
        , oldDuration(oldDuration)
        , newDuration(newDuration)
    {
    }

    bool execute() override
    {
        return applyFade(newDuration);
    }

    bool undo() override
    {
        return applyFade(oldDuration);
    }

    juce::String getDescription() const override
    {
        return isFadeIn ? "Fade In" : "Fade Out";
    }

    bool canMergeWith(const UndoableCommand* other) const override
    {
        auto* otherFade = dynamic_cast<const FadeClipCommand*>(other);
        if (!otherFade)
            return false;

        return otherFade->clipId == clipId &&
               otherFade->isFadeIn == isFadeIn &&
               (otherFade->timestamp - timestamp) < mergeWindowMs;
    }

    void mergeWith(const UndoableCommand* other) override
    {
        auto* otherFade = dynamic_cast<const FadeClipCommand*>(other);
        if (otherFade)
        {
            newDuration = otherFade->newDuration;
            timestamp = otherFade->timestamp;
        }
    }

private:
    bool applyFade(double duration)
    {
        if (auto* track = project->getTrack(trackId))
        {
            for (auto& clip : track->clips)
            {
                if (clip.id == clipId)
                {
                    if (isFadeIn)
                        clip.fadeInDuration = duration;
                    else
                        clip.fadeOutDuration = duration;

                    // Update audio engine
                    if (mixer)
                    {
                        for (int i = 0; i < mixer->getNumTracks(); ++i)
                        {
                            if (auto* at = mixer->getTrack(i))
                            {
                                if (at->getId() == trackId)
                                {
                                    at->updateClipFade(clipId, isFadeIn, duration);
                                    break;
                                }
                            }
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }

    Project* project;
    AudioMixer* mixer;
    juce::Uuid trackId;
    juce::Uuid clipId;
    bool isFadeIn;
    double oldDuration;
    double newDuration;
};
