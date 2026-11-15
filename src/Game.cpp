#include "Game.hpp"

#include "Log.hpp"
#include "Math.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>

Game::Game() : m_random(m_seed)
{
    m_noise = ValueNoise(m_random.derivedSeed("noise"));
    generateField();
    generateTerrain();
    generateDecor();

    m_playerEntity = spawnEntity("Player", {375.f, 275.f}, {50.f, 50.f},
                                 sf::Color(80, 200, 120), true);
    spawnEntity("Crate", {200.f, 150.f}, {40.f, 40.f}, sf::Color(220, 140, 60));
    spawnEntity("Crate", {560.f, 380.f}, {40.f, 40.f}, sf::Color(220, 140, 60));

    Log::info("Game created with seed " + std::to_string(m_seed));
}

std::size_t Game::spawnEntity(const std::string& name, sf::Vector2f position,
                              sf::Vector2f size, sf::Color color,
                              bool playerControlled)
{
    Entity entity;
    entity.name = name;
    entity.position = position;
    entity.size = size;
    entity.color = color;
    entity.playerControlled = playerControlled;
    m_entities.push_back(entity);
    Log::info("Entity spawned: " + name + " at (" +
              std::to_string(static_cast<int>(position.x)) + ", " +
              std::to_string(static_cast<int>(position.y)) + ")");
    return m_entities.size() - 1;
}

void Game::setSeed(std::uint32_t seed)
{
    if (seed == m_seed)
        Log::warning("Seed " + std::to_string(seed) +
                     " re-applied: the world regenerates identically");
    else
        Log::info("Seed set to " + std::to_string(seed));
    m_seed = seed;
    m_random.reseed(seed);
    m_noise = ValueNoise(m_random.derivedSeed("noise"));
    generateField();
    generateTerrain();
    generateDecor();
}

void Game::generateField()
{
    constexpr int tilesX = 40;
    constexpr int tilesY = 30;
    constexpr float tileSize = 20.f;

    m_field.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_field.clear();

    auto heightToColor = [](float h) {
        if (h < 0.35f) return sf::Color(40, 70, 140);   // deep water
        if (h < 0.45f) return sf::Color(60, 100, 170);  // water
        if (h < 0.50f) return sf::Color(200, 190, 130); // sand
        if (h < 0.65f) return sf::Color(90, 160, 80);   // grass
        if (h < 0.80f) return sf::Color(50, 110, 60);   // forest
        return sf::Color(200, 200, 205);                // rock / snow
    };

    for (int ty = 0; ty < tilesY; ++ty)
    {
        for (int tx = 0; tx < tilesX; ++tx)
        {
            float h = m_noise.fbm(tx * tileSize * m_noiseFrequency,
                                  ty * tileSize * m_noiseFrequency, 4);
            sf::Color c = heightToColor(h);
            float x = tx * tileSize;
            float y = ty * tileSize;

            // One tile = two triangles in a single batched vertex array.
            sf::Vertex v;
            v.color = c;
            v.position = {x, y};
            m_field.append(v);
            v.position = {x + tileSize, y};
            m_field.append(v);
            v.position = {x, y + tileSize};
            m_field.append(v);
            v.position = {x + tileSize, y};
            m_field.append(v);
            v.position = {x + tileSize, y + tileSize};
            m_field.append(v);
            v.position = {x, y + tileSize};
            m_field.append(v);
        }
    }

    std::ostringstream oss;
    oss << "Field generated: " << tilesX << 'x' << tilesY
        << " tiles, noise scale " << std::fixed << std::setprecision(3)
        << m_noiseFrequency;
    Log::info(oss.str());
}

void Game::setNoiseFrequency(float frequency)
{
    // Deliberately no log line here: the editor slider fires this
    // continuously while dragging, and the console would flood.
    m_noiseFrequency = frequency;
    generateField();
}

void Game::generateTerrain()
{
    m_blocks.clear();
    for (int i = 0; i < 12; ++i)
    {
        float size = m_random.rangeFloat("terrain", 20.f, 60.f);
        sf::RectangleShape block(sf::Vector2f(size, size));
        block.setPosition({m_random.rangeFloat("terrain", 0.f, 800.f - size),
                           m_random.rangeFloat("terrain", 0.f, 600.f - size)});
        int shade = m_random.rangeInt("terrain", 80, 130);
        block.setFillColor(sf::Color(static_cast<std::uint8_t>(shade),
                                     static_cast<std::uint8_t>(shade + 20),
                                     static_cast<std::uint8_t>(shade + 50)));
        m_blocks.push_back(block);
    }
    Log::info("Terrain generated: " + std::to_string(m_blocks.size()) + " blocks");
}

void Game::generateDecor()
{
    m_dots.clear();
    for (int i = 0; i < 20; ++i)
    {
        float r = m_random.rangeFloat("decor", 3.f, 7.f);
        sf::CircleShape dot(r);
        dot.setPosition({m_random.rangeFloat("decor", 0.f, 800.f - 2.f * r),
                         m_random.rangeFloat("decor", 0.f, 600.f - 2.f * r)});
        dot.setFillColor(sf::Color(220, 180, 60));
        m_dots.push_back(dot);
    }
    Log::info("Decor generated: " + std::to_string(m_dots.size()) + " dots");
}

void Game::update(float dt, sf::Vector2f worldSize)
{
    time += dt;

    sf::Vector2f dir(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        dir.x += 1.f;

    dir = math::normalize(dir);

    // Movement is a system operating on whichever entity the player
    // controls — the input code no longer knows a special shape exists.
    Entity& player = m_entities[m_playerEntity];
    sf::Vector2f pos = player.position + dir * speed * dt;
    pos.x = std::clamp(pos.x, 0.f, std::max(0.f, worldSize.x - player.size.x));
    pos.y = std::clamp(pos.y, 0.f, std::max(0.f, worldSize.y - player.size.y));
    player.position = pos;
}

void Game::render(sf::RenderTarget& target) const
{
    sf::Color clearCol(
        static_cast<std::uint8_t>(clearColor[0] * 255.f),
        static_cast<std::uint8_t>(clearColor[1] * 255.f),
        static_cast<std::uint8_t>(clearColor[2] * 255.f));

    target.clear(clearCol);
    target.draw(m_field);
    for (const auto& block : m_blocks)
        target.draw(block);
    for (const auto& dot : m_dots)
        target.draw(dot);

    // Entities are data; the rectangle is derived from the data each
    // frame. Two passes so the player always draws on top.
    sf::RectangleShape shape;
    for (int pass = 0; pass < 2; ++pass)
    {
        for (const auto& entity : m_entities)
        {
            if ((pass == 1) != entity.playerControlled)
                continue;
            shape.setSize(entity.size);
            shape.setPosition(entity.position);
            shape.setFillColor(entity.color);
            target.draw(shape);
        }
    }
}

sf::Vector2f Game::playerPosition() const
{
    return m_entities[m_playerEntity].position;
}
