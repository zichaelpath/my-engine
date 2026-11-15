#pragma once

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
