// Easing curves (Lesson 49) — pure functions from t in [0, 1] to
// progress, the vocabulary every tween in the engine speaks. Each
// satisfies f(0) = 0 and f(1) = 1 exactly; what happens in between
// is the personality. Header-only: they are small, hot, and shared
// by anything that animates (Game today, the editor tomorrow).
//
// The set is deliberately the working subset, not the whole
// easings.net catalogue: one linear, the quad/cubic staples, a
// back ease that overshoots (things arriving with a pop), and a
// bounce (things landing). More curves earn their place when a
// lesson needs them, not before.

#pragma once

#include <cmath>

namespace ease
{
using Fn = float (*)(float);

inline float linear(float t) { return t; }

inline float inQuad(float t) { return t * t; }

inline float outQuad(float t) { return 1.f - (1.f - t) * (1.f - t); }

inline float inOutQuad(float t)
{
    return t < 0.5f ? 2.f * t * t : 1.f - std::pow(-2.f * t + 2.f, 2.f) / 2.f;
}

inline float outCubic(float t) { return 1.f - std::pow(1.f - t, 3.f); }

// Overshoots above 1 mid-flight, then settles — the pop. The
// constant 1.70158 is the standard "how much overshoot" knob.
inline float outBack(float t)
{
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;
    const float u = t - 1.f;
    return 1.f + c3 * u * u * u + c1 * u * u;
}

// Bounces against 1 as it arrives — the landing. Piecewise, from
// the classic Penner equations.
inline float outBounce(float t)
{
    constexpr float n1 = 7.5625f;
    constexpr float d1 = 2.75f;
    if (t < 1.f / d1)
        return n1 * t * t;
    if (t < 2.f / d1)
    {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    }
    if (t < 2.5f / d1)
    {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    }
    t -= 2.625f / d1;
    return n1 * t * t + 0.984375f;
}
} // namespace ease
