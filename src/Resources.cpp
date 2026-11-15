#include "Resources.hpp"

#include "Log.hpp"

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
