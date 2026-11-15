#pragma once

#include <cstdint>
#include <map>
#include <random>
#include <string>

class Random
{
public:
    explicit Random(std::uint32_t masterSeed);

    void reseed(std::uint32_t masterSeed);
    std::uint32_t masterSeed() const {return m_masterSeed; }
    std::uint32_t derivedSeed(const std::string& streamName) const;

    // A seed derived from the master seed + a name, for subsystems (like
    // noise) that key off the master seed without consuming stream draws.
    std::uint32_t derivedSeed(const std::string& streamName) const;

    int rangeInt(const std::string& streamName, int minInclusive, int maxInclusive);
    float rangeFloat(const std::string& streamName, float minInclusive, float maxInclusive);
    bool chance(const std::string& streamName, float probability);

private:
    std::mt19937& stream(const std::string& name);
    static std::uint32_t hashName(const std::string& name);

    std::uint32_t m_masterSeed;
    std::map<std::string, std::mt19937> m_streams;

};