#include "TransportController.h"

TransportController::TransportController()
{
}

void TransportController::play()
{
    if (state.load() != State::Playing)
    {
        // If we were stopped (e.g., after reaching the end), reset to beginning
        if (state.load() == State::Stopped)
        {
            double dur = duration.load();
            double pos = position.load();
            // Reset position if we're at or past the end
            if (dur > 0.0 && pos >= dur)
            {
                position.store(0.0);
                notifyPositionChanged();
            }
        }

        state.store(State::Playing);
        notifyStateChanged();
    }
}

void TransportController::pause()
{
    if (state.load() == State::Playing)
    {
        state.store(State::Paused);
        notifyStateChanged();
    }
}

void TransportController::stop()
{
    state.store(State::Stopped);
    position.store(0.0);
    notifyStateChanged();
    notifyPositionChanged();
}

void TransportController::togglePlayPause()
{
    if (isPlaying())
        pause();
    else
        play();
}

void TransportController::setPosition(double timeInSeconds)
{
    position.store(juce::jmax(0.0, timeInSeconds));
    notifyPositionChanged();
}

double TransportController::getPosition() const
{
    return position.load();
}

TransportController::State TransportController::getState() const
{
    return state.load();
}

bool TransportController::isPlaying() const
{
    return state.load() == State::Playing;
}

bool TransportController::isPaused() const
{
    return state.load() == State::Paused;
}

bool TransportController::isStopped() const
{
    return state.load() == State::Stopped;
}

void TransportController::setTempo(double bpm)
{
    tempo.store(juce::jmax(20.0, juce::jmin(999.0, bpm)));
}

double TransportController::getTempo() const
{
    return tempo.load();
}

void TransportController::setDuration(double seconds)
{
    duration.store(juce::jmax(0.0, seconds));
}

double TransportController::getDuration() const
{
    return duration.load();
}

void TransportController::advancePosition(int numSamples, double sampleRate)
{
    if (isPlaying() && sampleRate > 0)
    {
        double advance = static_cast<double>(numSamples) / sampleRate;
        double newPos = position.load() + advance;

        // Stop at end of project
        double dur = duration.load();
        if (dur > 0.0 && newPos >= dur)
        {
            newPos = dur;
            state.store(State::Stopped);

            juce::MessageManager::callAsync([this]()
            {
                notifyStateChanged();
            });
        }

        position.store(newPos);
    }
}

void TransportController::addListener(Listener* listener)
{
    juce::ScopedLock lock(listenerLock);
    listeners.push_back(listener);
}

void TransportController::removeListener(Listener* listener)
{
    juce::ScopedLock lock(listenerLock);
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void TransportController::notifyStateChanged()
{
    sendChangeMessage();

    juce::ScopedLock lock(listenerLock);
    State currentState = state.load();
    for (auto* listener : listeners)
        listener->transportStateChanged(currentState);
}

void TransportController::notifyPositionChanged()
{
    juce::ScopedLock lock(listenerLock);
    double currentPos = position.load();
    for (auto* listener : listeners)
        listener->transportPositionChanged(currentPos);
}
