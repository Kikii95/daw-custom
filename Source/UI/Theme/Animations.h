#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <functional>

namespace Animations
{
    //=========================================================================
    // EASING FUNCTIONS
    //=========================================================================
    namespace Easing
    {
        // Linear
        inline float linear(float t) { return t; }

        // Quadratic
        inline float easeInQuad(float t) { return t * t; }
        inline float easeOutQuad(float t) { return t * (2.0f - t); }
        inline float easeInOutQuad(float t)
        {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }

        // Cubic
        inline float easeInCubic(float t) { return t * t * t; }
        inline float easeOutCubic(float t)
        {
            float t1 = t - 1.0f;
            return t1 * t1 * t1 + 1.0f;
        }
        inline float easeInOutCubic(float t)
        {
            return t < 0.5f
                ? 4.0f * t * t * t
                : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        }

        // Exponential
        inline float easeOutExpo(float t)
        {
            return t >= 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
        }

        // Elastic
        inline float easeOutElastic(float t)
        {
            if (t <= 0.0f) return 0.0f;
            if (t >= 1.0f) return 1.0f;
            return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.075f) * (2.0f * juce::MathConstants<float>::pi) / 0.3f) + 1.0f;
        }

        // Back (overshoot)
        inline float easeOutBack(float t)
        {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;
            float t1 = t - 1.0f;
            return 1.0f + c3 * t1 * t1 * t1 + c1 * t1 * t1;
        }
    }

    //=========================================================================
    // ANIMATED VALUE
    //=========================================================================
    /**
     * A value that smoothly transitions between states.
     * Uses JUCE's timer system for updates.
     */
    class AnimatedValue : public juce::Timer
    {
    public:
        using EasingFunction = std::function<float(float)>;

        AnimatedValue(float initialValue = 0.0f)
            : currentValue(initialValue), targetValue(initialValue), startValue(initialValue)
        {
        }

        ~AnimatedValue() override { stopTimer(); }

        void setValue(float newTarget, int durationMs = 200,
                      EasingFunction easing = Easing::easeOutCubic)
        {
            if (std::abs(newTarget - targetValue) < 0.001f)
                return;

            targetValue = newTarget;
            startValue = currentValue;
            easingFunc = easing;
            duration = durationMs;
            elapsed = 0;
            startTimer(16);  // ~60fps
        }

        void setValueImmediate(float newValue)
        {
            stopTimer();
            currentValue = newValue;
            targetValue = newValue;
            startValue = newValue;
            if (onValueChanged)
                onValueChanged(currentValue);
        }

        float getValue() const { return currentValue; }
        float getTarget() const { return targetValue; }
        bool isAnimating() const { return isTimerRunning(); }

        std::function<void(float)> onValueChanged;
        std::function<void()> onAnimationComplete;

    private:
        void timerCallback() override
        {
            elapsed += 16;
            float t = std::min(1.0f, static_cast<float>(elapsed) / static_cast<float>(duration));
            float easedT = easingFunc ? easingFunc(t) : t;

            currentValue = startValue + (targetValue - startValue) * easedT;

            if (onValueChanged)
                onValueChanged(currentValue);

            if (t >= 1.0f)
            {
                stopTimer();
                currentValue = targetValue;
                if (onAnimationComplete)
                    onAnimationComplete();
            }
        }

        float currentValue;
        float targetValue;
        float startValue;
        int duration = 200;
        int elapsed = 0;
        EasingFunction easingFunc = Easing::easeOutCubic;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedValue)
    };

    //=========================================================================
    // COLOUR ANIMATOR
    //=========================================================================
    /**
     * Smoothly interpolates between colours.
     */
    class ColourAnimator : public juce::Timer
    {
    public:
        ColourAnimator(juce::Colour initial = juce::Colours::black)
            : currentColour(initial), targetColour(initial), startColour(initial)
        {
        }

        ~ColourAnimator() override { stopTimer(); }

        void setColour(juce::Colour newTarget, int durationMs = 200)
        {
            if (newTarget == targetColour)
                return;

            targetColour = newTarget;
            startColour = currentColour;
            duration = durationMs;
            elapsed = 0;
            startTimer(16);
        }

        void setColourImmediate(juce::Colour newColour)
        {
            stopTimer();
            currentColour = newColour;
            targetColour = newColour;
            startColour = newColour;
            if (onColourChanged)
                onColourChanged(currentColour);
        }

        juce::Colour getColour() const { return currentColour; }
        bool isAnimating() const { return isTimerRunning(); }

        std::function<void(juce::Colour)> onColourChanged;

    private:
        void timerCallback() override
        {
            elapsed += 16;
            float t = std::min(1.0f, static_cast<float>(elapsed) / static_cast<float>(duration));
            t = Easing::easeOutCubic(t);

            currentColour = startColour.interpolatedWith(targetColour, t);

            if (onColourChanged)
                onColourChanged(currentColour);

            if (t >= 1.0f)
            {
                stopTimer();
                currentColour = targetColour;
            }
        }

        juce::Colour currentColour;
        juce::Colour targetColour;
        juce::Colour startColour;
        int duration = 200;
        int elapsed = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourAnimator)
    };

    //=========================================================================
    // FADE COMPONENT MIXIN
    //=========================================================================
    /**
     * Mixin for components that can fade in/out.
     * Use with multiple inheritance.
     */
    template <typename ComponentType>
    class Fadeable
    {
    public:
        void fadeIn(int durationMs = 200)
        {
            opacity.setValue(1.0f, durationMs);
            opacity.onValueChanged = [this](float val)
            {
                static_cast<ComponentType*>(this)->setAlpha(val);
                static_cast<ComponentType*>(this)->repaint();
            };
        }

        void fadeOut(int durationMs = 200)
        {
            opacity.setValue(0.0f, durationMs);
            opacity.onValueChanged = [this](float val)
            {
                static_cast<ComponentType*>(this)->setAlpha(val);
                static_cast<ComponentType*>(this)->repaint();
            };
        }

        float getOpacity() const { return opacity.getValue(); }

    protected:
        AnimatedValue opacity { 1.0f };
    };

    //=========================================================================
    // PULSE ANIMATION
    //=========================================================================
    /**
     * Creates a pulsing effect (scale up/down).
     */
    class PulseAnimator : public juce::Timer
    {
    public:
        PulseAnimator() = default;
        ~PulseAnimator() override { stopTimer(); }

        void pulse(float intensity = 0.1f, int durationMs = 300)
        {
            this->intensity = intensity;
            this->duration = durationMs;
            elapsed = 0;
            startTimer(16);
        }

        void stop()
        {
            stopTimer();
            scale = 1.0f;
        }

        float getScale() const { return scale; }

        std::function<void(float)> onScaleChanged;

    private:
        void timerCallback() override
        {
            elapsed += 16;
            float t = static_cast<float>(elapsed) / static_cast<float>(duration);

            if (t >= 1.0f)
            {
                stopTimer();
                scale = 1.0f;
            }
            else
            {
                // Pulse up then down
                float phase = t * 2.0f * juce::MathConstants<float>::pi;
                scale = 1.0f + intensity * std::sin(phase) * (1.0f - t);
            }

            if (onScaleChanged)
                onScaleChanged(scale);
        }

        float intensity = 0.1f;
        int duration = 300;
        int elapsed = 0;
        float scale = 1.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PulseAnimator)
    };
}
