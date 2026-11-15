#include "Random.hpp"

Random::Random(std::uint32_t masterSeed)
    : m_masterSeed(masterSeed)
{
}

void Random::reseed(std::uint32_t masterSeed)
{
    m_masterSeed = masterSeed;
    m_streams.clear();
}

std::uint32_t Random::hashName(const std::string& name)
{
    // FNV-1a: a tiny hash whose result is identical on every platform,
    // unlike std::hash<std::string>, which the standard leaves unspecified.
    std::uint32_t hash = 2166136261u;
    for (char c : name)
    {
        hash ^= static_cast<std::uint8_t>(c);
        hash *= 16777619u;
    }
    return hash;
}

std::uint32_t Random::derivedSeed(const std::string& streamName) const
{
    // Master seed + name hash, avalanched. Const and draw-free: asking for
    // a derived seed can never reshuffle any stream.
    std::uint32_t h = m_masterSeed ^ hashName(streamName);
    h ^= h >> 16;
    h *= 0x7FEB352Du;
    h ^= h >> 15;
    return h;
}

std::mt19937& Random::stream(const std::string& name)
{
    auto it = m_streams.find(name);
    if (it == m_streams.end())
    {
        std::seed_seq seed{m_masterSeed, hashName(name)};
        it = m_streams.emplace(name, std::mt19937(seed)).first;
    }
    return it->second;
}

int Random::rangeInt(const std::string& streamName, int minInclusive, int maxInclusive)
{
    std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
    return dist(stream(streamName));
}

float Random::rangeFloat(const std::string& streamName, float minInclusive, float maxInclusive)
{
    std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
    return dist(stream(streamName));
}

bool Random::chance(const std::string& streamName, float probability)
{
    return rangeFloat(streamName, 0.f, 1.f) < probability;
}

std::uint32_t Random::derivedSeed(const std::string& streamName) const 
{
    std::uint32_t h = m_masterSeed ^ hashName(streamName);
    h ^= h >> 16;
    h *= 0x7FEB352Du;
    h ^= h >> 15;
    return h;
}