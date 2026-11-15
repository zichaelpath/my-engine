#pragma once

#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <cmath>

// The engine's small math toolbox: the operations gameplay and procgen
// code reach for constantly, in one place, with the edge cases handled
// once. Header-only: these are one-liners the compiler should inline.
namespace math
{

// Linear interpolation: start at a, move t of the way toward b.
inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

// Map a value from one range onto another (0..1 height to 0..255
// colour, a sensor reading to a bar width, ...).
inline float remap(float value, float inMin, float inMax, float outMin, float outMax)
{
    float t = (value - inMin) / (inMax - inMin);
    return lerp(outMin, outMax, t);
}

// The smoothstep curve on 0..1: eases at both ends, no hard seams.
// Used to weight noise interpolation in Lesson 11.
inline float smoothstep01(float t)
{
    t = std::clamp(t, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

// Axis-aligned bounding box overlap: true if the two rectangles
// (position = top-left corner) share any area. Touching edges do
// not count as overlapping, which is what lets a mover rest exactly
// against a wall.
inline bool rectsOverlap(sf::Vector2f posA, sf::Vector2f sizeA,
                         sf::Vector2f posB, sf::Vector2f sizeB)
{
    return posA.x < posB.x + sizeB.x && posA.x + sizeA.x > posB.x &&
           posA.y < posB.y + sizeB.y && posA.y + sizeA.y > posB.y;
}

// Normalise a direction. SFML's Vector2::normalized() asserts on a
// zero vector (verified in the SFML 3.0.2 source), and "no input" is
// the most common direction a game ever sees, so the engine goes
// through here instead: a zero vector normalises to a zero vector.
inline sf::Vector2f normalize(sf::Vector2f v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    return len > 0.f ? v / len : sf::Vector2f(0.f, 0.f);
}

} // namespace math
