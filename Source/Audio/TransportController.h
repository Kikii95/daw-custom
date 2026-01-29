#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <atomic>
#include <vector>

class TransportController : public juce::ChangeBroadcaster
{
public:
    enum class State
    {
        Stopped,
        Playing,
        Paused
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void transportStateChanged(State newState) = 0;
        virtual void transportPositionChanged(double newPosition) {}
        virtual void transportLoopChanged(bool enabled, double start, double end) {}
        virtual void tempoChanged(double newBpm) {}
    };

    TransportController();
    ~TransportController() override = default;

    // Transport controls
    void play();
    void pause();
    void stop();
    void togglePlayPause();

    // Position
    void setPosition(double timeInSeconds);
    double getPosition() const;

    // State
    State getState() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    // Tempo (for future beat sync)
    void setTempo(double bpm);
    double getTempo() const;

    // Duration (total project length)
    void setDuration(double seconds);
    double getDuration() const;

    // Loop points
    void setLoopEnabled(bool enabled);
    bool isLoopEnabled() const;
    void setLoopStart(double timeInSeconds);
    void setLoopEnd(double timeInSeconds);
    double getLoopStart() const;
    double getLoopEnd() const;
    void setLoopRange(double start, double end);
    void clearLoop();

    // Playback speed (0.25x - 4x)
    void setPlaybackSpeed(double speed);
    double getPlaybackSpeed() const;

    // Reverse playback
    void setReversed(bool reversed);
    bool isReversed() const;

    // Called from audio thread to advance position
    void advancePosition(int numSamples, double sampleRate);

    // Listeners
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    void notifyStateChanged();
    void notifyPositionChanged();
    void notifyLoopChanged();
    void notifyTempoChanged();

    std::atomic<State> state { State::Stopped };
    std::atomic<double> position { 0.0 };
    std::atomic<double> tempo { 120.0 };
    std::atomic<double> duration { 0.0 };

    // Loop state
    std::atomic<bool> loopEnabled { false };
    std::atomic<double> loopStart { 0.0 };
    std::atomic<double> loopEnd { 0.0 };

    // Speed and direction
    std::atomic<double> playbackSpeed { 1.0 };
    std::atomic<bool> reversed { false };

    std::vector<Listener*> listeners;
    juce::CriticalSection listenerLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportController)
};
