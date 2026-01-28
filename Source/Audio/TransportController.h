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

    // Called from audio thread to advance position
    void advancePosition(int numSamples, double sampleRate);

    // Listeners
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    void notifyStateChanged();
    void notifyPositionChanged();

    std::atomic<State> state { State::Stopped };
    std::atomic<double> position { 0.0 };
    std::atomic<double> tempo { 120.0 };
    std::atomic<double> duration { 0.0 };

    std::vector<Listener*> listeners;
    juce::CriticalSection listenerLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportController)
};
