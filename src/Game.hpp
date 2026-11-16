#pragma once

#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include "Noise.hpp"
#include "Random.hpp"
#include "Particles.hpp"
#include "Resources.hpp"
#include "ScriptHost.hpp"
#include "Tween.hpp"
#include <cstddef>
#include <deque>
#include <filesystem>
#include <map>
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
    // Music (Lesson 30): one streamed track, looped. Unlike a
    // sound buffer, a stream decodes from disk as it plays — a
    // three-minute song costs kilobytes of memory, not megabytes.
    bool startMusic(const std::string& path);
    void stopMusic();
    bool musicPlaying() const;
    float musicOffset() const; // seconds into the track
    const std::string& musicTrack() const { return m_musicTrack; }
    // The game (Lesson 32, Arc 7): the decor dots are gold. Collect
    // them all to clear the world; the run timer runs until then.
    int goldCollected() const { return m_goldCollected; }
    int goldTotal() const { return m_goldTotal; }
    bool won() const { return m_won; }

    // The rising pressure (Lesson 74): Temple Run's run state.
    // pressureY is the doom line's world y — everything below it
    // is consumed. heightReached is the score: the highest the
    // player's feet have been above the ascent's floor.
    bool pressureActive() const { return m_pressureActive; }
    bool caught() const { return m_caught; }
    float pressureY() const { return m_pressureY; }
    float heightReached() const { return m_bestHeight; }
    float pressureSpeed = 26.f; // units/s, once the grace ends
    float pressureDelay = 2.5f; // seconds of grace at the foot
    float runTime() const { return m_runTime; }
    void newRun(std::uint32_t seed); // fresh world, fresh run

    // Stages the climber test shaft (Lesson 59): a floor, two
    // one-way ledges, a patrolling platform and a goal, with the
    // player placed at its base as a platformer. Rebuildable —
    // previous Climber entities are replaced, never duplicated.
    void buildClimberTest();

    // The chunk model (Lesson 73): authored vertical slices a
    // generator deals like cards. loadChunks reads definitions
    // (chunk=/height=/platform= lines); buildAscent replaces the
    // world with a climb — a floor, a platformer player, and
    // chunkCount chunks dealt from the seed (same seed, same
    // deal; a chunk never repeats back-to-back) stacked upward
    // in a shaft carved out of the tile field.
    bool loadChunks(const std::string& path);
    const std::vector<ChunkDef>& chunkDefs() const { return m_chunks; }
    void buildAscent(std::uint32_t seed, int chunkCount);
    const std::vector<std::string>& ascentDeal() const
    {
        return m_ascentDeal;
    }
    float ascentFloorY() const { return m_ascentFloorY; }
    float ascentTopY() const { return m_ascentTopY; }
    // The endless window (Lesson 75): how many chunks are live,
    // how many have ever been dealt, and how far the world has
    // scrolled down under the climber (the score's memory).
    std::size_t liveChunkCount() const { return m_liveChunks.size(); }
    int chunksDealtTotal() const { return m_nextChunkIndex; }
    float scrollTotal() const { return m_scrollTotal; }

    // The player's spawn-in pop (Lesson 49): 1 when settled, an
    // eased 0-to-1 while the run's first moments play out. Render
    // reads it; tests measure it.
    float playerSpawnScale() const { return m_spawnPop.value(); }

    // Camera feel (Lesson 51). Trauma is the shake budget: events
    // add it, time spends it, and the offset it produces scales
    // with trauma squared — small rattles whisper, big hits shout.
    // Hosts add shakeOffset() to their view centre every frame.
    void addTrauma(float amount);
    float trauma() const { return m_trauma; }
    sf::Vector2f shakeOffset() const;
    // Hit-stop: freeze the world for a beat. The stop clock runs
    // on real dt; everything else in update() waits.
    void hitStop(float seconds);
    bool isHitStopped() const { return m_hitStop > 0.f; }
    // The player's actual post-collision velocity, for camera
    // lookahead (and, in time, for scripts).
    sf::Vector2f playerVelocity() const { return m_playerVel; }

    // Tilemap queries (Lesson 21): the tile id at a world position
    // (-1 outside the world), the tile under the player's feet, and
    // a display name for an id.
    int tileAtWorld(sf::Vector2f worldPos) const;

    // Tile solidity (Lesson 56): which tiles are matter to a
    // platformer. The top-down model never consults this — its
    // world is a ground plane it walks over; the platformer's is
    // a side view where some tiles are stone. Rock ships solid;
    // the table is data, so a project can re-geologize later.
    bool tileSolid(int id) const
    {
        return id >= 0 && id < 6 && m_tileSolid[id];
    }
    void setTileSolid(int id, bool solid)
    {
        if (id >= 0 && id < 6)
            m_tileSolid[id] = solid;
    }
    int playerTile() const;
    static const char* tileName(int id);
    // Grid-space access (Lesson 38): dimensions, the id at a grid
    // cell (-1 outside), and the one palette both renderers share —
    // hoisted out of the 2D field builder so the 3D heightfield is
    // coloured by the same table, not a copy of it.
    int tilesX() const { return m_tilesX; }
    int tilesY() const { return m_tilesY; }
    int tileAt(int tx, int ty) const;
    static sf::Color tileColor(int id);

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

    // Entity types (Lesson 68): the registry of type definitions.
    // defineType registers or replaces one; applyType stamps a
    // type's defaults onto an entity (and records the type name);
    // spawnTyped creates an entity from a type in one call;
    // loadTypes reads definitions from a text file in the
    // engine's key=value family — a `type=` line opens a block,
    // the keys below it fill the definition.
    void defineType(const EntityType& typeDef);
    const EntityType* findType(const std::string& name) const;
    bool applyType(Entity& entity, const std::string& typeName) const;
    bool spawnTyped(const std::string& typeName, const std::string& name,
                    sf::Vector2f position);
    bool loadTypes(const std::string& path);
    const std::vector<EntityType>& types() const { return m_types; }

    // Scripts in the loop (Lesson 70): the Game owns a ScriptHost.
    // Script paths on entities are project-relative; setScriptRoot
    // says where they resolve (empty = the working directory).
    // bindScripts (re)binds an instance for every scripted entity
    // — called when a scene loads, a run starts, and on demand.
    void setScriptRoot(const std::string& root) { m_scriptRoot = root; }
    const std::string& scriptRoot() const { return m_scriptRoot; }
    void bindScripts();
    // Hot reload (Lesson 71): if a bound script's file changed on
    // disk, reload its module and rebind only that script's
    // instances (their state resets; other scripts run on
    // untouched). Returns how many modules were reloaded. Hosts
    // poll this on a throttle; the harness calls it directly.
    std::size_t reloadScriptsIfChanged();
    ScriptHost& scriptHost() { return m_scripts; }
    std::size_t scriptBindingCount() const
    {
        return m_scriptBindings.size();
    }

    // Scene serialisation (Lesson 18): the world is a recipe — seed +
    // noise scale regenerate the terrain exactly (Arc 2 determinism),
    // so the file only needs the recipe and the entity records.
    bool saveScene(const std::string& path) const;
    bool loadScene(const std::string& path);

    // Scene travel (Lesson 78): any entity named "Exit:<scene>"
    // is a door — when the player's hitbox overlaps it, the
    // target is latched here for the host to act on (hosts own
    // files; Game owns the touching). Loading a scene clears it.
    // placePlayerAtSpawn moves the player to the entity named
    // "Spawn:<fromScene>" — or plain "Spawn" — in the current
    // scene; false when the scene declares neither.
    std::string pendingExit() const { return m_pendingExit; }
    void clearPendingExit() { m_pendingExit.clear(); }
    bool placePlayerAtSpawn(const std::string& fromScene);
    // Shipping hosts set this: Spawn markers are authoring aids
    // and stay invisible in the shipped game. The editor leaves
    // it false — an author must see where spawns are.
    bool hideSpawnMarkers = false;

    float speed = 200.f;

    // Platformer physics (Lesson 54): the second movement model's
    // numbers, public data like speed — Arc 3's Physics panel will
    // edit them live, and the harness pins their behaviour.
    float gravity = 1800.f;         // units/s^2
    float terminalVelocity = 900.f; // units/s, the fall's ceiling
    float platformerSpeed = 230.f;  // run speed, units/s
    float jumpSpeed = 620.f;        // the jump's launch velocity (L55)
    float coyoteTime = 0.10f;       // grace after leaving ground (L55)
    float jumpBuffer = 0.12f;       // grace before touching it (L55)
    float jumpCut = 0.45f;          // release early, rise this much (L55)

    // The input seam (Lesson 55): update() reads the keyboard —
    // unless an override is set, in which case it reads this. Tests
    // drive a player with it today; scripts and AI will tomorrow.
    struct InputState
    {
        sf::Vector2f move{0.f, 0.f};
        bool jump = false;
    };
    void setInput(const InputState& in)
    {
        m_input = in;
        m_inputOverride = true;
    }
    void clearInput() { m_inputOverride = false; }
    float clearColor[3] = {30.f / 255.f, 30.f / 255.f, 40.f / 255.f};
    bool showHitboxes = false; // editor debug overlay (Lesson 19)

    // Every game-feel constant in one struct (Lesson 53): the
    // editor's Feel panel edits this live, the harness pins its
    // defaults, and a game that wants a different personality
    // changes data, not code. Defaults ARE the shipped feel —
    // Lessons 49-52 were tuned into these numbers.
    struct Feel
    {
        bool enabled = true;          // the whole layer, one switch
        bool particlesEnabled = true; // bursts, splashes, drips, rings
        float popDuration = 0.35f;    // spawn-in pop, seconds (L49)
        float shakeMax = 14.f;        // offset at trauma 1.0 (L51)
        float traumaDecay = 1.4f;     // trauma spent per second (L51)
        float hitStopWin = 0.12f;     // the win's frozen beat (L52)
        float streakWindow = 1.2f;    // seconds to keep a streak (L52)
        int burstBase = 10;           // pickup burst size (L52)
        int burstPerStreak = 2;       // ... plus this per streak step
    };
    Feel feel;

    // Fires an emitter through the game's own pool and RNG — the
    // same call the collection and splash paths make, and the one
    // scripts will get in Arc 5.
    void spawnParticles(const Emitter& emitter);
    std::size_t particleCount() const { return m_particles.activeCount(); }

    // The juice hooks (Lesson 52). The collection loop calls the
    // first for every dot and the second once, at the win; they are
    // public because a feel event the tests (and later, scripts)
    // cannot fire is a feel event nobody can verify.
    void onGoldCollected(sf::Vector2f at);
    void onRunWon();
    int collectStreak() const { return m_collectStreak; }
    std::size_t ringCount() const { return m_rings.size(); }
    float time = 0.f;

private:
    // Pushes the mover out of every collider along one axis, back to
    // exact contact. Called once per axis per frame (Lesson 15).
    void resolveCollisions(Entity& mover, float movement, bool xAxis);
    void updatePlatforms(float dt); // patrol motion (Lesson 57)

    // The endless deal (Lesson 75): dealChunkOnTop deals one
    // chunk onto the stack's top from the run's persistent RNG;
    // maintainAscent (called from update) pops consumed chunks,
    // scrolls the world down by their height, and refills the top.
    std::string dealChunkOnTop();
    void maintainAscent();

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
    std::vector<EntityType> m_types; // the type registry (Lesson 68)

    // The ascent (Lesson 73): the chunk vocabulary, the last
    // deal (chunk names, bottom to top), and the climb's extent.
    std::vector<ChunkDef> m_chunks;
    std::vector<std::string> m_ascentDeal;
    float m_ascentFloorY = 0.f;
    float m_ascentTopY = 0.f;

    // Pressure state (Lesson 74): armed only in an ascent.
    bool m_pressureActive = false;
    bool m_caught = false;
    float m_pressureY = 0.f;
    float m_pressureTimer = 0.f;
    float m_bestHeight = 0.f;

    // The endless window (Lesson 75): the live chunks, bottom
    // first; the run's own dealer (reseeded by buildAscent, drawn
    // for the run's whole life); and the accumulated scroll —
    // every unit the world has shifted down under the player,
    // which is height the score must never forget.
    struct LiveChunk
    {
        int index = 0;
        std::string name;
        float topY = 0.f;
        float height = 0.f;
    };
    std::deque<LiveChunk> m_liveChunks;
    Random m_ascentRng{0};
    int m_ascentLastPick = -1;
    int m_nextChunkIndex = 0;
    float m_scrollTotal = 0.f;
    float m_shaftLeft = 0.f;

    // Scripting (Lesson 70): the owned host, the root script
    // paths resolve against, the module cache (resolved path ->
    // module id), and the live bindings. A binding latches off
    // (errored) after its first failure until the next bind.
    ScriptHost m_scripts;
    std::string m_scriptRoot;
    std::map<std::string, int> m_scriptModules;
    std::map<std::string, std::filesystem::file_time_type>
        m_scriptModuleTimes; // file mtime at load (Lesson 71)
    struct ScriptBinding
    {
        std::string entityName;
        int instanceId = -1;
        bool errored = false;
    };
    std::vector<ScriptBinding> m_scriptBindings;
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
    sf::Music m_music;
    std::string m_musicTrack;
    float m_bumpCooldown = 0.f; // bump-sound throttle (Lesson 31)
    FloatTween m_spawnPop;      // spawn-in scale pop (Lesson 49)
    ParticleSystem m_particles; // the feel layer's confetti (Lesson 50)
    float m_trauma = 0.f;       // camera shake budget (Lesson 51)
    float m_hitStop = 0.f;      // world-freeze clock (Lesson 51)
    sf::Vector2f m_playerVel;   // post-collision velocity (Lesson 51)

    // Lesson 52 juice state: the collection streak (pickups close
    // together escalate the burst), shockwave rings, and the win
    // sequence's fountain clock.
    struct Ring
    {
        sf::Vector2f position;
        float life = 0.f;
        float maxLife = 0.28f;
        float maxRadius = 26.f;
        sf::Color color = sf::Color::White;
    };
    std::vector<Ring> m_rings;
    int m_collectStreak = 0;
    float m_streakTimer = 0.f;
    float m_winFxTimer = 0.f;
    float m_winFxAccum = 0.f;
    int m_winFxBursts = 0; // fountain colour alternation
    int m_goldCollected = 0;
    int m_goldTotal = 0;
    bool m_won = false;
    std::string m_pendingExit; // a touched Exit's target (L78)
    float m_runTime = 0.f;

    // Input seam + jump state (Lesson 55)
    InputState m_input;
    bool m_inputOverride = false;
    bool m_prevJump = false;   // last frame's jump button (edges)
    float m_coyoteTimer = 0.f; // time since ground was true
    float m_bufferTimer = 0.f; // time since jump was pressed
    bool m_jumpCutDone = true; // variable-height cut already spent

    std::vector<int> m_tiles;
    bool m_tileSolid[6] = {false, false, false, false, false, true}; // Rock
    int m_tilesX = 0;
    int m_tilesY = 0;
    std::vector<sf::RectangleShape> m_blocks;
    std::vector<sf::CircleShape> m_dots;
    sf::VertexArray m_field;
};
