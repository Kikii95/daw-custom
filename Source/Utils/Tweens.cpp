#include "Tweens.h"

namespace Tweening
{
    EasingFunction GetFunction(EasingType type)
    {
        switch(type)
        {
            case EasingType::Linear:            return Linear;

            case EasingType::EaseInSine:        return SineIn;
            case EasingType::EaseOutSine:       return SineOut;
            case EasingType::EaseInOutSine:     return SineInOut;

            case EasingType::EaseInCubic:       return CubicIn;
            case EasingType::EaseOutCubic:      return CubicOut;
            case EasingType::EaseInOutCubic:    return CubicInOut;

            case EasingType::EaseInQuint:       return QuintIn;
            case EasingType::EaseOutQuint:      return QuintOut;
            case EasingType::EaseInOutQuint:    return QuintInOut;

            case EasingType::EaseInCirc:        return CircIn;
            case EasingType::EaseOutCirc:       return CircOut;
            case EasingType::EaseInOutCirc:     return CircInOut;

            case EasingType::EaseInElastic:     return ElasticIn;
            case EasingType::EaseOutElastic:    return ElasticOut;
            case EasingType::EaseInOutElastic:  return ElasticInOut;

            case EasingType::EaseInQuad:        return QuadIn;
            case EasingType::EaseOutQuad:       return QuadOut;
            case EasingType::EaseInOutQuad:     return QuadInOut;

            case EasingType::EaseInQuart:       return QuartIn;
            case EasingType::EaseOutQuart:      return QuartOut;
            case EasingType::EaseInOutQuart:    return QuartInOut;

            case EasingType::EaseInExpo:        return ExpoIn;
            case EasingType::EaseOutExpo:       return ExpoOut;
            case EasingType::EaseInOutExpo:     return ExpoInOut;

            case EasingType::EaseInBack:        return BackIn;
            case EasingType::EaseOutBack:       return BackOut;
            case EasingType::EaseInOutBack:     return BackInOut;

            case EasingType::EaseInBounce:      return BounceIn;
            case EasingType::EaseOutBounce:     return BounceOut;
            case EasingType::EaseInOutBounce:   return BounceInOut;

            default: return Linear;
        }
    }

    float Linear(float t)       { return t; }

    float SineIn(float t)       { return 1.0f - std::cos((t * PI) / 2.0f); }
    float SineOut(float t)      { return std::sin((t * PI) / 2.0f); }
    float SineInOut(float t)    { return -(std::cos(PI * t) - 1.0f) / 2.0f; }

    float CubicIn(float t)      { return t * t * t; }
    float CubicOut(float t)     { t -= 1.0f; return t * t * t + 1.0f; }
    float CubicInOut(float t)   { return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3) / 2.0f; }

    float QuintIn(float t)      { return t * t * t * t * t; }
    float QuintOut(float t)     { return std::sqrt(1 - std::pow(t - 1, 2)); }
    float QuintInOut(float t)   { return t < 0.5 ? 16 * t * t * t * t * t : 1 - std::pow(-2 * t + 2, 5) / 2; }

    float CircIn(float t)       { return 1 - std::sqrt(1 - std::pow(t, 2)); }
    float CircOut(float t)      { return std::sqrt(1 - std::pow(t - 1, 2)); }
    float CircInOut(float t)    { return t < 0.5 ? (1 - std::sqrt(1 - std::pow(2 * t, 2))) / 2  : (std::sqrt(1 - std::pow(-2 * t + 2, 2)) + 1) / 2; }

    float ElasticIn(float t)
    {
        float c4 = (2 * PI) / 3;
        return t == 0 ? 0 : t == 1 ? 1 : - std::pow(2, 10 * t - 10) * std::sin((t * 10 - 10.75) * c4);
    }
    float ElasticOut(float t)
    {
        float c4 = (2 * PI) / 3;
        return t == 0 ? 0 : t == 1 ? 1 : std::pow(2, -10 * t) * std::sin((t * 10 - 0.75) * c4) + 1;
    }
    float ElasticInOut(float t)
    {
        float c5 = (2 * PI) / 4.5;
        return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? -(std::pow(2, 20 * t - 10) * std::sin((20 * t - 11.125) * c5)) / 2 : (std::pow(2, -20 * t + 10) * std::sin((20 * t - 11.125) * c5)) / 2 + 1;
    }

    float QuadIn(float t)       { return t * t; }
    float QuadOut(float t)      { return 1 - (1 - t) * (1 - t);  }
    float QuadInOut(float t)    { return t < 0.5f ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2; }

    float QuartIn(float t)      { return t * t * t * t; }
    float QuartOut(float t)     { return 1 - std::pow(t - 1, 4); }
    float QuartInOut(float t)   { return t < 0.5f ? 8 * t * t * t * t : 1 - std::pow(-2 * t + 2, 4) / 2; }

    float ExpoIn(float t)       { return t == 0 ? 0 : std::pow(2, 10 * t - 10); }
    float ExpoOut(float t)      { return t == 1 ? 1 : 1 - std::pow(2, -10 * t); }
    float ExpoInOut(float t)    { return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? std::pow(2, 20 * t - 10) / 2 : (2 - std::pow(2, -20 * t + 10)) / 2; }

    float BackIn(float t)
    {
        float c1 = 1.70158f;
        float c3 = c1 + 1;
        return c3 * t * t * t - c1 * t * t;
    }
    float BackOut(float t)
    {
        float c1 = 1.70158f;
        float c3 = c1 + 1;
        return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
    }
    float BackInOut(float t)
    {
        float c1 = 1.70158f;
        float c2 = c1 * 1.525f;
        return t < 0.5 ? (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2  : (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
    }

    float BounceIn(float t)     { return 1 - BounceOut(1 - t); }
    float BounceOut(float t)
    {
        float n1 = 7.5625f;
        float d1 = 2.75f;

        if (t < 1 / d1) return n1 * t * t;
        if (t < 2 / d1) return n1 * (t -= 1.5f / d1) * t + 0.75f;
        if (t < 2.5f / d1) return n1 * (t -= 2.25f / d1) * t + 0.9375f;

        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }
    float BounceInOut(float t)  { return t < 0.5 ? (1 - BounceOut(1 - 2 * t)) / 2 : (1 + BounceOut(2 * t - 1)) / 2; }
}
