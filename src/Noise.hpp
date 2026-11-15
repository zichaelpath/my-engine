#pragma once

#include <cstdint>

// Value noise: smooth pseudo-random variation over 2D space.
// Lattice values are hashed from (seed, x, y) on demand — never stored —
// so the field is infinite, order-independent, and identical everywhere.
class ValueNoise
{
public:
    explicit ValueNoise(std::uint32_t seed) : m_seed(seed) {}

    // One octave of value noise at (x, y); result in [0, 1).
    float sample(float x, float y) const;

    // Octaves layered: large shapes plus progressively finer detail.
    // Result in [0, 1).
    float fbm(float x, float y, int octaves) const;

private:
    float lattice(int x, int y) const;

    std::uint32_t m_seed;
};
