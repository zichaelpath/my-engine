#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <map>
#include <string>

// Owns every file-loaded texture and hands out references to them.
// First request for a path loads it; later requests return the cached
// texture. A failed load produces the shared placeholder (a magenta
// checkerboard — the industry-standard "missing texture" livery) and
// is cached too, so a missing file is reported once, not per frame.
//
// Two deliberate choices live here:
//  - std::map, because its node storage never invalidates references
//    on insertion — callers hold references across later loads.
//  - get() is const with a mutable cache: loading on demand does not
//    change the cache's observable state, only its contents
//    ("logical constness" — the standard cache pattern).
class TextureCache
{
public:
    TextureCache();

    const sf::Texture& get(const std::string& path) const;
    std::size_t size() const { return m_textures.size(); }

private:
    mutable std::map<std::string, sf::Texture> m_textures;
    sf::Image m_placeholderImage;
};

// The sound twin of TextureCache (Lesson 29): same load-once
// contract, same map, same logical constness. The placeholder is a
// short synthesized blip, so a missing sound file still makes *a*
// sound — and the error is logged once, like textures.
class SoundBufferCache
{
public:
    SoundBufferCache();

    const sf::SoundBuffer& get(const std::string& path) const;
    std::size_t size() const { return m_buffers.size(); }

private:
    mutable std::map<std::string, sf::SoundBuffer> m_buffers;
    sf::SoundBuffer m_placeholder;
};
