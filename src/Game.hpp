#pragma once

#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include "Noise.hpp"
#include "Random.hpp"
#include "Resources.hpp"
#include <cstddef>
#include <string>
#include <vector>

class Game
{
public:
    Game();

    void update(float dt);
    void render(sf::RenderTarget& target) const;
    void setSeed(std::uint32_t seed);
    void generateTerrain();
    void generateDecor();
    void generateField();
    void setNoiseFrequency(float frequency);
    std::uint32_t seed() const { return m_seed; }
    float noiseFrequency() const { return m_noiseFrequency; }
    sf::Vector2f playerPosition() const;
    sf::Vector2f playerCenter() const;
    sf::Vector2f worldSize() const { return m_worldSize; }
    int playerFrame() const { return m_entities[m_playerEntity].frame; }
    std::size_t textureCacheSize() const { return m_resources.size(); }

    // Adds an entity to the world and returns its index (its identity:
    // indices stay valid for the world's lifetime; entities are never
    // removed yet, which is what makes that promise safe today).
    std::size_t spawnEntity(const std::string& name, sf::Vector2f position,
                            sf::Vector2f size, sf::Color color,
                            bool playerControlled = false, int sprite = -1);
    const std::vector<Entity>& entities() const { return m_entities; }

    // Scene serialisation (Lesson 18): the world is a recipe — seed +
    // noise scale regenerate the terrain exactly (Arc 2 determinism),
    // so the file only needs the recipe and the entity records.
    bool saveScene(const std::string& path) const;
    bool loadScene(const std::string& path);

    float speed = 200.f;
    float clearColor[3] = {30.f / 255.f, 30.f / 255.f, 40.f / 255.f};
    bool showHitboxes = false; // editor debug overlay (Lesson 19)
    float time = 0.f;

private:
    // Pushes the mover out of every collider along one axis, back to
    // exact contact. Called once per axis per frame (Lesson 15).
    void resolveCollisions(Entity& mover, float movement, bool xAxis);

    // Rebuilds the random streams and regenerates field, terrain and
    // decor from the current seed — shared by setSeed and loadScene.
    void regenerateWorld();

    // Declaration order is initialisation order: m_seed must be declared
    // before m_random, whose constructor reads it.
    std::uint32_t m_seed = 12345;
    Random m_random;
    ValueNoise m_noise{0};
    float m_noiseFrequency = 0.01f;

    // The world's size is the game's own fact (Lesson 16): generation,
    // movement clamps, and camera clamps all read this — never the
    // size of any canvas the world happens to be drawn on.
    sf::Vector2f m_worldSize{1600.f, 1200.f};

    std::vector<Entity> m_entities;
    std::size_t m_playerEntity = 0;
    sf::Texture m_sheet;
    TextureCache m_resources;
    std::vector<sf::RectangleShape> m_blocks;
    std::vector<sf::CircleShape> m_dots;
    sf::VertexArray m_field;
};
