#include "Noise.hpp"

#include "Math.hpp"

#include <cmath>

namespace
{
// Integer finaliser (splitmix-style): avalanches input bits so that
// neighbouring inputs produce unrelated outputs.
std::uint32_t mix32(std::uint32_t v)
{
    v ^= v >> 16;
    v *= 0x7FEB352Du;
    v ^= v >> 15;
    v *= 0x846CA68Bu;
    v ^= v >> 16;
    return v;
}
} // namespace

float ValueNoise::lattice(int x, int y) const
{
    std::uint32_t h = m_seed;
    h ^= static_cast<std::uint32_t>(x) * 0x9E3779B1u;
    h ^= static_cast<std::uint32_t>(y) * 0x85EBCA6Bu;
    return static_cast<float>(mix32(h)) / 4294967296.f;
}

float ValueNoise::sample(float x, float y) const
{
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    float tx = math::smoothstep01(x - static_cast<float>(xi));
    float ty = math::smoothstep01(y - static_cast<float>(yi));

    float topLeft = lattice(xi, yi);
    float topRight = lattice(xi + 1, yi);
    float bottomLeft = lattice(xi, yi + 1);
    float bottomRight = lattice(xi + 1, yi + 1);

    float top = math::lerp(topLeft, topRight, tx);
    float bottom = math::lerp(bottomLeft, bottomRight, tx);
    return math::lerp(top, bottom, ty);
}

float ValueNoise::fbm(float x, float y, int octaves) const
{
    float total = 0.f;
    float weight = 1.f;
    float weightSum = 0.f;
    float frequency = 1.f;
    for (int i = 0; i < octaves; ++i)
    {
        total += sample(x * frequency, y * frequency) * weight;
        weightSum += weight;
        weight *= 0.5f;
        frequency *= 2.f;
    }
    return total / weightSum;
}
