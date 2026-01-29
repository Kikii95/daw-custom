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
    double newTempo = juce::jmax(20.0, juce::jmin(999.0, bpm));
    if (std::abs(tempo.load() - newTempo) > 0.01)
    {
        tempo.store(newTempo);
        notifyTempoChanged();
    }
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

void TransportController::setLoopEnabled(bool enabled)
{
    loopEnabled.store(enabled);
    notifyLoopChanged();
}

bool TransportController::isLoopEnabled() const
{
    return loopEnabled.load();
}

void TransportController::setLoopStart(double timeInSeconds)
{
    loopStart.store(juce::jmax(0.0, timeInSeconds));
    notifyLoopChanged();
}

void TransportController::setLoopEnd(double timeInSeconds)
{
    double dur = duration.load();
    double maxEnd = dur > 0.0 ? dur : timeInSeconds;
    loopEnd.store(juce::jmax(0.0, juce::jmin(timeInSeconds, maxEnd)));
    notifyLoopChanged();
}

double TransportController::getLoopStart() const
{
    return loopStart.load();
}

double TransportController::getLoopEnd() const
{
    return loopEnd.load();
}

void TransportController::setLoopRange(double start, double end)
{
    if (end > start)
    {
        loopStart.store(juce::jmax(0.0, start));
        loopEnd.store(juce::jmax(0.0, end));
        notifyLoopChanged();
    }
}

void TransportController::clearLoop()
{
    loopEnabled.store(false);
    loopStart.store(0.0);
    loopEnd.store(0.0);
    notifyLoopChanged();
}

void TransportController::setPlaybackSpeed(double speed)
{
    playbackSpeed.store(juce::jlimit(0.25, 4.0, speed));
}

double TransportController::getPlaybackSpeed() const
{
    return playbackSpeed.load();
}

void TransportController::setReversed(bool rev)
{
    reversed.store(rev);
}

bool TransportController::isReversed() const
{
    return reversed.load();
}

void TransportController::advancePosition(int numSamples, double sampleRate)
{
    if (isPlaying() && sampleRate > 0)
    {
        double speed = playbackSpeed.load();
        bool rev = reversed.load();
        double direction = rev ? -1.0 : 1.0;

        double advance = (static_cast<double>(numSamples) / sampleRate) * speed * direction;
        double newPos = position.load() + advance;
        double dur = duration.load();

        // Check loop
        bool loop = loopEnabled.load();
        double lStart = loopStart.load();
        double lEnd = loopEnd.load();

        if (loop && lEnd > lStart)
        {
            // Loop handling (forward)
            if (!rev && newPos >= lEnd)
            {
                newPos = lStart + std::fmod(newPos - lStart, lEnd - lStart);
            }
            // Loop handling (reverse)
            else if (rev && newPos < lStart)
            {
                double range = lEnd - lStart;
                newPos = lEnd - std::fmod(lStart - newPos, range);
            }
        }
        else if (!rev && dur > 0.0 && newPos >= dur)
        {
            // Stop at end of project (forward, no loop)
            newPos = dur;
            state.store(State::Stopped);

            juce::MessageManager::callAsync([this]()
            {
                notifyStateChanged();
            });
        }
        else if (rev && newPos < 0.0)
        {
            // Stop at beginning (reverse, no loop)
            newPos = 0.0;
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

void TransportController::notifyLoopChanged()
{
    juce::ScopedLock lock(listenerLock);
    bool enabled = loopEnabled.load();
    double start = loopStart.load();
    double end = loopEnd.load();
    for (auto* listener : listeners)
        listener->transportLoopChanged(enabled, start, end);
}

void TransportController::notifyTempoChanged()
{
    juce::ScopedLock lock(listenerLock);
    double currentTempo = tempo.load();
    for (auto* listener : listeners)
        listener->tempoChanged(currentTempo);
}
