// FloatTween (Lesson 49) — one float, carried from `from` to `to`
// over `duration` seconds along an easing curve. The smallest
// tween that is still a system: Game advances it with dt, asks for
// the current value, and never thinks about curves itself.
//
// A tween that has not been started reports `from`; one that has
// finished reports `to` forever (clamped, no drift). Zero or
// negative duration means "already there" — start() completes it
// immediately rather than dividing by zero later.

#pragma once

#include "Easing.hpp"

struct FloatTween
{
    float from = 0.f;
    float to = 1.f;
    float duration = 0.f;
    float elapsed = 0.f;
    ease::Fn curve = ease::linear;
    bool running = false;

    void start(float fromValue, float toValue, float seconds, ease::Fn easeFn)
    {
        from = fromValue;
        to = toValue;
        duration = seconds;
        curve = easeFn;
        elapsed = 0.f;
        running = duration > 0.f;
    }

    // Advances the tween; returns true while it is still running.
    bool update(float dt)
    {
        if (!running)
            return false;
        elapsed += dt;
        if (elapsed >= duration)
        {
            elapsed = duration;
            running = false;
        }
        return running;
    }

    float value() const
    {
        if (duration <= 0.f)
            return to;
        const float t = elapsed / duration;
        return from + (to - from) * curve(t);
    }

    bool done() const { return !running; }
};
