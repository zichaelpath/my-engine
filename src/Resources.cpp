#include "Resources.hpp"

#include "Log.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

TextureCache::TextureCache()
{
    // The placeholder: a 32x32 magenta/black checkerboard, built once
    // as an image so every failed texture can be stamped from it.
    m_placeholderImage = sf::Image({32, 32}, sf::Color::Black);
    for (unsigned y = 0; y < 32; ++y)
        for (unsigned x = 0; x < 32; ++x)
            if ((x / 8 + y / 8) % 2 == 0)
                m_placeholderImage.setPixel({x, y}, sf::Color::Magenta);
}

const sf::Texture& TextureCache::get(const std::string& path) const
{
    auto it = m_textures.find(path);
    if (it != m_textures.end())
        return it->second;

    auto [inserted, _] = m_textures.try_emplace(path);
    if (inserted->second.loadFromFile(path))
    {
        Log::info("Texture loaded: " + path);
        return inserted->second;
    }
    Log::error("Texture failed to load: " + path + " (placeholder used)");
    inserted->second.loadFromImage(m_placeholderImage);
    return inserted->second;
}

SoundBufferCache::SoundBufferCache()
{
    // The placeholder blip: 0.12 s of 660 Hz sine with a decay
    // envelope, synthesized once — a missing sound is still audible.
    constexpr unsigned rate = 44100;
    const std::size_t count = rate * 12 / 100;
    std::vector<std::int16_t> samples(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(rate);
        float env = 1.f - static_cast<float>(i) / static_cast<float>(count);
        samples[i] = static_cast<std::int16_t>(
            12000.f * env * std::sin(2.f * 3.14159265f * 660.f * t));
    }
    m_placeholder.loadFromSamples(samples.data(), count, 1, rate,
                                  {sf::SoundChannel::Mono});
}

const sf::SoundBuffer& SoundBufferCache::get(const std::string& path) const
{
    auto it = m_buffers.find(path);
    if (it != m_buffers.end())
        return it->second;

    auto [inserted, _] = m_buffers.try_emplace(path);
    if (inserted->second.loadFromFile(path))
    {
        Log::info("Sound loaded: " + path);
        return inserted->second;
    }
    Log::error("Sound failed to load: " + path + " (placeholder blip used)");
    inserted->second = m_placeholder;
    return inserted->second;
}
