#pragma once

#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include "Noise.hpp"
#include "Random.hpp"
#include "Resources.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// Tile ids for the world tilemap (Lesson 21). The noise field is
// classified into these once at generation; rendering, movement and
// (later) editing all read the ids, never the raw noise.
namespace tiles
{
constexpr int DeepWater = 0;
constexpr int Water = 1;
constexpr int Sand = 2;
constexpr int Grass = 3;
constexpr int Forest = 4;
constexpr int Rock = 5;
} // namespace tiles

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
    // One-shot sounds (Lesson 29): played through a small pool of
    // reusable players fed by the sound cache.
    void playSound(const std::string& path, float pitch = 1.f);
    int soundsPlayed() const { return m_soundsPlayed; }
    std::size_t soundCacheSize() const { return m_soundBuffers.size(); }

    // Tilemap queries (Lesson 21): the tile id at a world position
    // (-1 outside the world), the tile under the player's feet, and
    // a display name for an id.
    int tileAtWorld(sf::Vector2f worldPos) const;
    int playerTile() const;
    static const char* tileName(int id);

    // Adds an entity to the world and returns its index (its identity:
    // indices stay valid for the world's lifetime; entities are never
    // removed yet, which is what makes that promise safe today).
    std::size_t spawnEntity(const std::string& name, sf::Vector2f position,
                            sf::Vector2f size, sf::Color color,
                            bool playerControlled = false, int sprite = -1);
    // Remove an entity by index (Lesson 26). Refuses the player —
    // the game depends on it. Indices above the removed one shift
    // down, and m_playerEntity follows its entity.
    void removeEntity(std::size_t index);
    std::size_t playerIndex() const { return m_playerEntity; }
    // Replace the whole entity list (editor undo/redo, Lesson 27).
    // The player index is re-derived from the playerControlled flag,
    // so a restored list can never point the game at the wrong entity.
    void replaceEntities(const std::vector<Entity>& entities);
    const std::vector<Entity>& entities() const { return m_entities; }
    // Mutable access for the editor's inspector (Lesson 24). The
    // runtime never calls this; it exists so tools can edit the
    // scene the game is simulating.
    std::vector<Entity>& entities() { return m_entities; }

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
    SoundBufferCache m_soundBuffers;
    std::vector<std::unique_ptr<sf::Sound>> m_players; // playback pool
    std::size_t m_nextPlayer = 0;
    int m_soundsPlayed = 0;
    float m_stepTimer = 0.f; // footstep cadence clock
    bool m_stepAlt = false;  // footstep pitch alternation
    int m_lastPlayerTile = -1; // splash edge detection (-1 = unknown)

    std::vector<int> m_tiles;
    int m_tilesX = 0;
    int m_tilesY = 0;
    std::vector<sf::RectangleShape> m_blocks;
    std::vector<sf::CircleShape> m_dots;
    sf::VertexArray m_field;
};
