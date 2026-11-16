#include "Game.hpp"

#include "Log.hpp"
#include "Math.hpp"
#include "SpriteSheet.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
constexpr float TILE_SIZE = 20.f;

// The collision size of an entity: its hitbox if it has one,
// otherwise the whole entity box.
sf::Vector2f hitboxSizeOf(const Entity& e)
{
    return e.hitboxSize.x > 0.f ? e.hitboxSize : e.size;
}

// Hitboxes are a property of the character type for now — the player
// sprite gets the feet box. Per-instance hitboxes arrive with the
// Arc 5 inspector, and the scene format grows fields then.
void applySpriteHitbox(Entity& e)
{
    if (e.sprite == sprites::PLAYER)
    {
        e.hitboxOffset = {11.f, 28.f};
        e.hitboxSize = {28.f, 22.f};
    }
}
} // namespace

Game::Game() : m_random(m_seed)
{
    // The sound playback pool (Lesson 29): eight slots, filled
    // lazily — SFML 3's sf::Sound is born with a buffer, so a slot
    // is created by its first sound. Eight overlapping one-shots is
    // generous for footsteps and splashes.
    m_players.resize(8);

    if (m_sheet.loadFromImage(buildSpriteSheetImage()))
        Log::info("Sprite sheet built in code: " +
                  std::to_string(m_sheet.getSize().x) + "x" +
                  std::to_string(m_sheet.getSize().y));
    else
        Log::error("Sprite sheet failed to build; entities draw as rectangles");

    m_noise = ValueNoise(m_random.derivedSeed("noise"));
    generateField();
    generateTerrain();
    generateDecor();

    m_playerEntity = spawnEntity("Player", {375.f, 275.f}, {50.f, 50.f},
                                 sf::Color(80, 200, 120), true, sprites::PLAYER);
    spawnEntity("Crate", {200.f, 150.f}, {40.f, 40.f}, sf::Color(220, 140, 60),
                false, sprites::CRATE);
    spawnEntity("Crate", {560.f, 380.f}, {40.f, 40.f}, sf::Color(220, 140, 60),
                false, sprites::CRATE);

    Log::info("Game created with seed " + std::to_string(m_seed));
}

std::size_t Game::spawnEntity(const std::string& name, sf::Vector2f position,
                              sf::Vector2f size, sf::Color color,
                              bool playerControlled, int sprite)
{
    Entity entity;
    entity.name = name;
    entity.position = position;
    entity.size = size;
    entity.color = color;
    entity.playerControlled = playerControlled;
    entity.sprite = sprite;
    applySpriteHitbox(entity);
    m_entities.push_back(entity);
    Log::info("Entity spawned: " + name + " at (" +
              std::to_string(static_cast<int>(position.x)) + ", " +
              std::to_string(static_cast<int>(position.y)) + ")");
    return m_entities.size() - 1;
}

void Game::removeEntity(std::size_t index)
{
    if (index >= m_entities.size())
        return;
    if (index == m_playerEntity)
    {
        Log::warning("Refused to remove the player entity");
        return;
    }
    Log::info("Entity removed: " + m_entities[index].name + " (index " +
              std::to_string(index) + ")");
    m_entities.erase(m_entities.begin() + static_cast<std::ptrdiff_t>(index));
    if (m_playerEntity > index)
        --m_playerEntity; // the player slid down with everyone else
}

void Game::replaceEntities(const std::vector<Entity>& entities)
{
    m_entities = entities;
    m_playerEntity = 0;
    for (std::size_t i = 0; i < m_entities.size(); ++i)
        if (m_entities[i].playerControlled)
        {
            m_playerEntity = i;
            break;
        }
}

void Game::setSeed(std::uint32_t seed)
{
    if (seed == m_seed)
        Log::warning("Seed " + std::to_string(seed) +
                     " re-applied: the world regenerates identically");
    else
        Log::info("Seed set to " + std::to_string(seed));
    m_seed = seed;
    regenerateWorld();
}

void Game::regenerateWorld()
{
    m_random.reseed(m_seed);
    m_noise = ValueNoise(m_random.derivedSeed("noise"));
    generateField();
    generateTerrain();
    generateDecor();
    // A regenerated world is a fresh run (Lesson 32).
    m_goldCollected = 0;
    m_won = false;
    m_runTime = 0.f;
}

void Game::newRun(std::uint32_t seed)
{
    if (seed != m_seed)
        setSeed(seed); // regenerates (and resets the run) itself
    else
        regenerateWorld();
    Log::info("New run: seed " + std::to_string(m_seed) + " - collect " +
              std::to_string(m_goldTotal) + " gold");
    // Arrive like you mean it: the player pops in from nothing,
    // overshooting slightly before settling (Lesson 49).
    if (feel.enabled)
        m_spawnPop.start(0.f, 1.f, feel.popDuration, ease::outBack);
}

void Game::generateField()
{
    m_tilesX = static_cast<int>(m_worldSize.x / TILE_SIZE);
    m_tilesY = static_cast<int>(m_worldSize.y / TILE_SIZE);

    // Step 1: classify the noise into tile ids — DATA first.
    auto classify = [](float h) {
        if (h < 0.35f) return tiles::DeepWater;
        if (h < 0.45f) return tiles::Water;
        if (h < 0.50f) return tiles::Sand;
        if (h < 0.65f) return tiles::Grass;
        if (h < 0.80f) return tiles::Forest;
        return tiles::Rock;
    };
    m_tiles.assign(static_cast<std::size_t>(m_tilesX * m_tilesY), tiles::Grass);
    for (int ty = 0; ty < m_tilesY; ++ty)
        for (int tx = 0; tx < m_tilesX; ++tx)
        {
            float h = m_noise.fbm(tx * TILE_SIZE * m_noiseFrequency,
                                  ty * TILE_SIZE * m_noiseFrequency, 4);
            m_tiles[static_cast<std::size_t>(ty * m_tilesX + tx)] = classify(h);
        }

    // Step 2: build the picture FROM the data. Id -> colour lives in
    // exactly one place (Game::tileColor), so the map and its legend
    // can never drift.
    m_field.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_field.clear();
    for (int ty = 0; ty < m_tilesY; ++ty)
    {
        for (int tx = 0; tx < m_tilesX; ++tx)
        {
            sf::Color c = tileColor(m_tiles[static_cast<std::size_t>(ty * m_tilesX + tx)]);
            float x = tx * TILE_SIZE;
            float y = ty * TILE_SIZE;

            // One tile = two triangles in a single batched vertex array.
            sf::Vertex v;
            v.color = c;
            v.position = {x, y};
            m_field.append(v);
            v.position = {x + TILE_SIZE, y};
            m_field.append(v);
            v.position = {x, y + TILE_SIZE};
            m_field.append(v);
            v.position = {x + TILE_SIZE, y};
            m_field.append(v);
            v.position = {x + TILE_SIZE, y + TILE_SIZE};
            m_field.append(v);
            v.position = {x, y + TILE_SIZE};
            m_field.append(v);
        }
    }

    std::ostringstream oss;
    oss << "Field generated: " << m_tilesX << 'x' << m_tilesY
        << " tiles, noise scale " << std::fixed << std::setprecision(3)
        << m_noiseFrequency;
    Log::info(oss.str());
}

int Game::tileAtWorld(sf::Vector2f worldPos) const
{
    int tx = static_cast<int>(std::floor(worldPos.x / TILE_SIZE));
    int ty = static_cast<int>(std::floor(worldPos.y / TILE_SIZE));
    return tileAt(tx, ty);
}

int Game::tileAt(int tx, int ty) const
{
    if (tx < 0 || ty < 0 || tx >= m_tilesX || ty >= m_tilesY)
        return -1;
    return m_tiles[static_cast<std::size_t>(ty * m_tilesX + tx)];
}

// The one palette (Lesson 38 hoisted it here from the field
// builder): the 2D vertex field and the 3D heightfield both read
// their colours from this table.
sf::Color Game::tileColor(int id)
{
    switch (id)
    {
        case tiles::DeepWater: return sf::Color(40, 70, 140);
        case tiles::Water:     return sf::Color(60, 100, 170);
        case tiles::Sand:      return sf::Color(200, 190, 130);
        case tiles::Grass:     return sf::Color(90, 160, 80);
        case tiles::Forest:    return sf::Color(50, 110, 60);
        default:               return sf::Color(200, 200, 205); // rock
    }
}

int Game::playerTile() const
{
    const Entity& player = m_entities[m_playerEntity];
    sf::Vector2f feet =
        player.position + player.hitboxOffset + hitboxSizeOf(player) / 2.f;
    return tileAtWorld(feet);
}

const char* Game::tileName(int id)
{
    switch (id)
    {
        case tiles::DeepWater: return "Deep water";
        case tiles::Water:     return "Water";
        case tiles::Sand:      return "Sand";
        case tiles::Grass:     return "Grass";
        case tiles::Forest:    return "Forest";
        case tiles::Rock:      return "Rock";
        default:               return "Void";
    }
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
    for (int i = 0; i < 24; ++i)
    {
        float size = m_random.rangeFloat("terrain", 20.f, 60.f);
        sf::RectangleShape block(sf::Vector2f(size, size));
        block.setPosition({m_random.rangeFloat("terrain", 0.f, m_worldSize.x - size),
                           m_random.rangeFloat("terrain", 0.f, m_worldSize.y - size)});
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
    for (int i = 0; i < 40; ++i)
    {
        float r = m_random.rangeFloat("decor", 3.f, 7.f);
        sf::CircleShape dot(r);
        dot.setPosition({m_random.rangeFloat("decor", 0.f, m_worldSize.x - 2.f * r),
                         m_random.rangeFloat("decor", 0.f, m_worldSize.y - 2.f * r)});
        dot.setFillColor(sf::Color(220, 180, 60));
        m_dots.push_back(dot);
    }
    m_goldTotal = static_cast<int>(m_dots.size());
    Log::info("Decor generated: " + std::to_string(m_dots.size()) + " dots");
}

void Game::addTrauma(float amount)
{
    if (!feel.enabled)
        return;
    m_trauma = std::clamp(m_trauma + amount, 0.f, 1.f);
}

sf::Vector2f Game::shakeOffset() const
{
    const float s = m_trauma * m_trauma; // quadratic: feel, not noise
    if (!feel.enabled || s <= 0.f)
        return {0.f, 0.f};
    // Two incommensurate sines: periodic enough to be smooth,
    // irregular enough to read as a rattle rather than a sway. A
    // pure function of game time — no RNG, so tests can reason
    // about it exactly.
    return {feel.shakeMax * s * std::sin(time * 39.7f),
            feel.shakeMax * 0.8f * s * std::cos(time * 47.9f)};
}

void Game::hitStop(float seconds)
{
    if (!feel.enabled)
        return;
    m_hitStop = std::max(m_hitStop, seconds);
}

void Game::update(float dt)
{
    // Hit-stop (Lesson 51): the world holds its breath. Only the
    // stop clock runs; even game time freezes, so a shake caught
    // mid-rattle resumes exactly where it stopped. A stop that
    // ends mid-frame hands its leftover dt to the world — time is
    // never silently eaten.
    if (m_hitStop > 0.f)
    {
        const float spent = std::min(m_hitStop, dt);
        m_hitStop -= spent;
        dt -= spent;
        if (dt <= 0.f)
            return;
    }
    time += dt;
    m_trauma = std::max(0.f, m_trauma - dt * feel.traumaDecay);
    m_spawnPop.update(dt);
    m_particles.update(dt);

    // Lesson 52 clocks: the streak window, the rings, and the win
    // fountain (which starts only once the hit-stop has released
    // the world — this code sits below the gate on purpose).
    if (m_streakTimer > 0.f)
    {
        m_streakTimer -= dt;
        if (m_streakTimer <= 0.f)
            m_collectStreak = 0;
    }
    for (std::size_t i = m_rings.size(); i-- > 0;)
    {
        m_rings[i].life -= dt;
        if (m_rings[i].life <= 0.f)
        {
            m_rings[i] = m_rings.back();
            m_rings.pop_back();
        }
    }
    if (m_winFxTimer > 0.f)
    {
        m_winFxTimer -= dt;
        m_winFxAccum += dt;
        while (m_winFxAccum >= 0.08f)
        {
            m_winFxAccum -= 0.08f;
            Emitter fountain;
            fountain.position =
                playerCenter() +
                sf::Vector2f(m_random.rangeFloat("fx", -26.f, 26.f),
                             m_random.rangeFloat("fx", -14.f, 14.f));
            fountain.count = 9;
            fountain.color = (++m_winFxBursts % 2 == 0)
                                 ? sf::Color(255, 205, 60)
                                 : sf::Color(255, 240, 200);
            fountain.speedMin = 60.f;
            fountain.speedMax = 150.f;
            spawnParticles(fountain);
        }
    }

    // Input (Lesson 55): the keyboard — unless the seam overrides
    // it. Jump is Space, plus W/Up for a platformer player, whose
    // vertical axis belongs to gravity anyway.
    sf::Vector2f dir(0.f, 0.f);
    bool jumpHeld = false;
    if (m_inputOverride)
    {
        dir = m_input.move;
        jumpHeld = m_input.jump;
    }
    else
    {
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
        jumpHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
                   (m_entities[m_playerEntity].moveModel ==
                        MoveModel::Platformer &&
                    (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                     sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)));
    }
    const bool jumpPressed = jumpHeld && !m_prevJump;
    m_prevJump = jumpHeld;

    // Platforms move first (Lesson 57), and a grounded rider is
    // carried by the platform under his feet — before his own
    // movement runs, so his gravity re-earns the ground after.
    updatePlatforms(dt);
    {
        Entity& rider = m_entities[m_playerEntity];
        if (rider.moveModel == MoveModel::Platformer && rider.grounded)
        {
            const sf::Vector2f rSize = hitboxSizeOf(rider);
            const float feet =
                rider.position.y + rider.hitboxOffset.y + rSize.y;
            const float rLeft = rider.position.x + rider.hitboxOffset.x;
            const float rRight = rLeft + rSize.x;
            for (const auto& p : m_entities)
            {
                if (!p.platform || &p == &rider)
                    continue;
                const float top = p.position.y + p.hitboxOffset.y;
                const float pLeft = p.position.x + p.hitboxOffset.x;
                if (std::abs(feet - top) < 1.f && rRight > pLeft &&
                    rLeft < pLeft + hitboxSizeOf(p).x)
                {
                    rider.position += p.frameDelta;
                    break;
                }
            }
        }
    }

    dir = math::normalize(dir);
    if (m_won)
        dir = {0.f, 0.f}; // the world stays alive after the win; the
                          // player takes his bow and stands still (L52)

    // Movement is a system operating on whichever entity the player
    // controls — the input code no longer knows a special shape exists.
    Entity& player = m_entities[m_playerEntity];
    const sf::Vector2f prevPlayerPos = player.position; // for velocity (L51)

    // Animation is the movement system's other output: facing follows
    // horizontal input, and the walk clock runs only while moving —
    // stop, and the character settles back to the idle frame.
    if (dir.x != 0.f)
        player.flipX = dir.x < 0.f;
    if (dir.x != 0.f || dir.y != 0.f)
    {
        player.animTime += dt;
        player.frame = static_cast<int>(player.animTime / 0.12f) % 4;
    }
    else
    {
        player.animTime = 0.f;
        player.frame = 0;
    }

    // The tile under the player's feet sets the pace (Lesson 21 —
    // the tilemap's first gameplay job): wading is slow, sand and
    // high rock drag, grass is full speed.
    float terrainFactor = 1.f;
    switch (playerTile())
    {
        case tiles::DeepWater: terrainFactor = 0.45f; break;
        case tiles::Water:     terrainFactor = 0.55f; break;
        case tiles::Sand:      terrainFactor = 0.85f; break;
        case tiles::Forest:    terrainFactor = 0.90f; break;
        case tiles::Rock:      terrainFactor = 0.70f; break;
        default: break;
    }

    sf::Vector2f move;
    if (player.moveModel == MoveModel::Platformer)
    {
        // Platformer (Lesson 54): input steers X, gravity owns Y.
        // The velocity is the state; this frame's move derives from
        // it, and the shared axis-separated resolver below decides
        // what survives — one collision truth for both models.
        //
        // Jump feel (Lesson 55): two grace timers working opposite
        // sides of the same moment. Coyote time forgives a press
        // just AFTER the ground vanishes; the buffer forgives one
        // just BEFORE it arrives. Each covers the other's blind
        // spot — including the first frame of standing, when
        // grounded is not yet true but a buffered press is waiting.
        if (player.grounded)
            m_coyoteTimer = coyoteTime;
        else
            m_coyoteTimer -= dt;
        if (jumpPressed)
            m_bufferTimer = jumpBuffer;
        else
            m_bufferTimer -= dt;
        if (!m_won && m_bufferTimer > 0.f && m_coyoteTimer > 0.f)
        {
            player.velocity.y = -jumpSpeed;
            m_bufferTimer = 0.f;
            m_coyoteTimer = 0.f;
            m_jumpCutDone = false;
        }
        // Variable height: release the button while still rising
        // and the rise is cut — once per jump.
        if (!jumpHeld && !m_jumpCutDone && player.velocity.y < 0.f)
        {
            player.velocity.y *= jumpCut;
            m_jumpCutDone = true;
        }
        player.velocity.x = dir.x * platformerSpeed;
        player.velocity.y =
            std::min(player.velocity.y + gravity * dt, terminalVelocity);
        move = player.velocity * dt;
    }
    else
    {
        move = dir * (speed * terrainFactor) * dt;
    }

    // Axis-separated collision: move and resolve one axis at a time.
    // A blocked axis never stops the other, so walls slide instead of
    // sticking. At this engine's speeds (a few pixels per frame) a
    // discrete overlap test cannot tunnel; fast projectiles will one
    // day need swept tests, and will say so when they arrive.
    sf::Vector2f beforeMove = player.position;
    if (move.x != 0.f)
    {
        player.position.x += move.x;
        resolveCollisions(player, move.x, true);
    }
    if (move.y != 0.f)
    {
        player.position.y += move.y;
        resolveCollisions(player, move.y, false);
    }

    // Bump (Lesson 31): if an axis asked for more travel than the
    // world allowed, the player ran into something. The displacement
    // comparison sees every collider (entities and terrain blocks
    // resolve through the same path), and a cooldown keeps a held
    // push from machine-gunning the sound.
    m_bumpCooldown -= dt;
    bool blockedX = move.x != 0.f &&
                    std::abs(player.position.x - beforeMove.x) < std::abs(move.x) - 0.001f;
    bool blockedY = move.y != 0.f &&
                    std::abs(player.position.y - beforeMove.y) < std::abs(move.y) - 0.001f;
    // A platformer's blocked Y is a landing, not a bump — it gets
    // its own treatment in Lesson 55 and stays silent here.
    const bool bumpWorthy =
        blockedX || (blockedY && player.moveModel == MoveModel::TopDown);
    if (bumpWorthy && m_bumpCooldown <= 0.f)
    {
        playSound("assets/bump.wav");
        addTrauma(0.15f); // a bump you can see, not just hear (Lesson 51)
        m_bumpCooldown = 0.25f;
    }

    // The world bounds hold the hitbox, not the sprite: feet stop at
    // the edge; the hood may lean past it.
    sf::Vector2f boxSize = hitboxSizeOf(player);
    player.position.x = std::clamp(
        player.position.x, -player.hitboxOffset.x,
        std::max(-player.hitboxOffset.x,
                 m_worldSize.x - player.hitboxOffset.x - boxSize.x));
    player.position.y = std::clamp(
        player.position.y, -player.hitboxOffset.y,
        std::max(-player.hitboxOffset.y,
                 m_worldSize.y - player.hitboxOffset.y - boxSize.y));

    // Grounded (Lesson 54): the platformer was falling, asked for
    // more downward travel than the world allowed, and so is
    // standing on whatever stopped it — a crate, a block, the
    // floor. A head-bonk on the way up spends the rise the same
    // way. Standing is re-earned every frame: gravity always asks
    // again, the ground always answers.
    if (player.moveModel == MoveModel::Platformer)
    {
        const float actualDy = player.position.y - beforeMove.y;
        const bool wasGrounded = player.grounded;
        player.grounded =
            player.velocity.y > 0.f && actualDy < move.y - 0.001f;
        // A heavy landing announces itself (Lesson 55): dust at
        // the feet and a whisper of trauma. Light landings — a
        // hop's return, a step down — stay silent on purpose.
        if (player.grounded && !wasGrounded && player.velocity.y > 480.f)
        {
            Emitter dust;
            dust.position = player.position + player.hitboxOffset +
                            sf::Vector2f(hitboxSizeOf(player).x / 2.f,
                                         hitboxSizeOf(player).y);
            dust.count = 8;
            dust.color = sf::Color(196, 178, 150);
            dust.speedMin = 25.f;
            dust.speedMax = 70.f;
            dust.gravity = 120.f;
            dust.lifeMin = 0.3f;
            dust.lifeMax = 0.5f;
            spawnParticles(dust);
            addTrauma(0.08f);
        }
        if (player.grounded ||
            (player.velocity.y < 0.f && actualDy > move.y + 0.001f))
            player.velocity.y = 0.f;
    }

    // Footsteps (Lesson 29): while the player walks, a step every
    // 0.27 s, alternating pitch so the loop doesn't sound mechanical.
    if (dir.x != 0.f || dir.y != 0.f)
    {
        m_stepTimer += dt;
        if (m_stepTimer >= 0.27f)
        {
            m_stepTimer = 0.f;
            playSound("assets/step.wav", m_stepAlt ? 1.06f : 0.94f);
            m_stepAlt = !m_stepAlt;
        }
    }
    else
        m_stepTimer = 0.20f; // the next walk's first step lands quickly

    // The player's true velocity this frame (post-collision), for
    // the camera's lookahead (Lesson 51).
    m_playerVel = dt > 0.0001f ? (player.position - prevPlayerPos) / dt
                               : sf::Vector2f(0.f, 0.f);

    // Splash: entering water from land, once per entry — an edge
    // detector over the tile stream, not a per-frame state.
    int tileNow = playerTile();
    auto isWater = [](int t) {
        return t == tiles::Water || t == tiles::DeepWater;
    };
    if (m_lastPlayerTile != -1 && isWater(tileNow) && !isWater(m_lastPlayerTile))
    {
        playSound("assets/splash.wav");
        Log::info("Splash - the player wades in");
        // The splash has had a sound since Lesson 29; since Lesson
        // 50 it has a picture too — a fountain at the player's feet.
        Emitter splash;
        splash.position = player.position + player.hitboxOffset +
                          sf::Vector2f(hitboxSizeOf(player).x / 2.f,
                                       hitboxSizeOf(player).y);
        splash.count = 14;
        splash.color = sf::Color(170, 210, 255);
        splash.speedMin = 50.f;
        splash.speedMax = 120.f;
        splash.spreadRad = 1.9f; // a cone, not a burst
        splash.lifeMin = 0.3f;
        splash.lifeMax = 0.55f;
        splash.sizeMin = 2.f;
        splash.sizeMax = 3.f;
        splash.gravity = 420.f;
        spawnParticles(splash);
    }
    // And the exit (Lesson 52): leaving water drips — a smaller,
    // sadder fountain at the same feet.
    if (m_lastPlayerTile != -1 && !isWater(tileNow) && isWater(m_lastPlayerTile))
    {
        Emitter drip;
        drip.position = player.position + player.hitboxOffset +
                        sf::Vector2f(hitboxSizeOf(player).x / 2.f,
                                     hitboxSizeOf(player).y);
        drip.count = 6;
        drip.color = sf::Color(150, 190, 240);
        drip.speedMin = 15.f;
        drip.speedMax = 45.f;
        drip.spreadRad = 2.4f;
        drip.lifeMin = 0.25f;
        drip.lifeMax = 0.45f;
        drip.sizeMin = 1.5f;
        drip.sizeMax = 2.5f;
        drip.gravity = 300.f;
        spawnParticles(drip);
    }
    m_lastPlayerTile = tileNow;

    // Gold collection (Lesson 32): the run timer runs until the
    // world is cleared; a dot is collected when its centre comes
    // within reach of the player's feet (the hitbox centre).
    if (!m_won)
        m_runTime += dt;
    sf::Vector2f feet = player.position + player.hitboxOffset +
                        hitboxSizeOf(player) / 2.f;
    for (std::size_t i = m_dots.size(); i-- > 0;)
    {
        float r = m_dots[i].getRadius();
        sf::Vector2f centre = m_dots[i].getPosition() + sf::Vector2f(r, r);
        sf::Vector2f d = centre - feet;
        float reach = r + 16.f;
        if (d.x * d.x + d.y * d.y < reach * reach)
        {
            m_dots[i] = m_dots.back(); // swap-with-last removal
            m_dots.pop_back();
            ++m_goldCollected;
            onGoldCollected(centre); // burst + ring + streak (L50/L52)
            playSound("assets/pickup.wav",
                      1.f + 0.02f * static_cast<float>(m_goldCollected % 8));
            if (m_goldCollected >= m_goldTotal && m_goldTotal > 0)
            {
                m_won = true;
                onRunWon(); // the staged sequence (Lesson 52)
                playSound("assets/win.wav");
                Log::info("World cleared in " + std::to_string(m_runTime) +
                          " s - all " + std::to_string(m_goldTotal) +
                          " gold collected");
            }
        }
    }
}

void Game::onGoldCollected(sf::Vector2f at)
{
    // The streak (Lesson 52): pickups within 1.2 s of each other
    // escalate — the burst grows with the streak, so a flowing run
    // *looks* different from a plodding one.
    ++m_collectStreak;
    m_streakTimer = feel.streakWindow;
    Emitter burst;
    burst.position = at;
    burst.count = feel.burstBase + feel.burstPerStreak * std::min(m_collectStreak, 8);
    burst.color = sf::Color(255, 205, 60);
    spawnParticles(burst);
    if (feel.enabled && feel.particlesEnabled)
    {
        Ring ring;
        ring.position = at;
        ring.life = ring.maxLife;
        ring.color = sf::Color(255, 205, 60);
        m_rings.push_back(ring);
    }
}

void Game::onRunWon()
{
    // The staged win (Lesson 52): first the freeze and the rattle
    // (Lesson 51), then — once the world breathes again — a
    // fountain over the player for most of a second, and a wide
    // ring to close it.
    hitStop(feel.hitStopWin);
    addTrauma(0.35f);
    m_winFxTimer = 0.7f;
    m_winFxAccum = 0.f;
    m_winFxBursts = 0;
    if (feel.enabled && feel.particlesEnabled)
    {
        Ring ring;
        ring.position = playerCenter();
        ring.life = ring.maxLife = 0.45f;
        ring.maxRadius = 60.f;
        ring.color = sf::Color(255, 225, 120);
        m_rings.push_back(ring);
    }
}

void Game::spawnParticles(const Emitter& emitter)
{
    if (!feel.enabled || !feel.particlesEnabled)
        return;
    // Cosmetic randomness only: the "fx" stream is derived from the
    // seed independently of every generation stream (Lesson 10's
    // design), so a burst can never change the world it decorates.
    m_particles.spawn(emitter, m_random);
}

void Game::playSound(const std::string& path, float pitch)
{
    const sf::SoundBuffer& buffer = m_soundBuffers.get(path);
    auto start = [&](std::unique_ptr<sf::Sound>& slot) {
        if (!slot)
            slot = std::make_unique<sf::Sound>(buffer); // born with it (SFML 3)
        else
        {
            slot->stop();
            slot->setBuffer(buffer);
        }
        slot->setPitch(pitch);
        slot->play();
        ++m_soundsPlayed;
    };

    // Round-robin over the pool: prefer a free player; if every slot
    // is busy, steal the next one (the oldest sound loses).
    for (std::size_t tries = 0; tries < m_players.size(); ++tries)
    {
        std::size_t i = (m_nextPlayer + tries) % m_players.size();
        if (!m_players[i] ||
            m_players[i]->getStatus() != sf::Sound::Status::Playing)
        {
            m_nextPlayer = (i + 1) % m_players.size();
            start(m_players[i]);
            return;
        }
    }
    std::size_t i = m_nextPlayer;
    m_nextPlayer = (m_nextPlayer + 1) % m_players.size();
    start(m_players[i]);
}

bool Game::startMusic(const std::string& path)
{
    if (!m_music.openFromFile(path))
    {
        // Music is garnish, not a dependency: one error, then silence.
        Log::error("Music failed to open: " + path + " (continuing silently)");
        return false;
    }
    m_music.setLooping(true);
    m_music.play();
    m_musicTrack = path;
    Log::info("Music started: " + path);
    return true;
}

void Game::stopMusic()
{
    m_music.stop();
    Log::info("Music stopped");
}

bool Game::musicPlaying() const
{
    return m_music.getStatus() == sf::Sound::Status::Playing;
}

float Game::musicOffset() const
{
    return m_music.getPlayingOffset().asSeconds();
}

void Game::updatePlatforms(float dt)
{
    for (auto& e : m_entities)
    {
        e.frameDelta = {0.f, 0.f};
        if (!e.platform)
            continue;
        if (!e.platformInit)
        {
            e.platformHome = e.position;
            e.platformInit = true;
        }
        if (e.platformSpeed <= 0.f)
            continue;
        const sf::Vector2f target =
            e.platformSign > 0 ? e.platformTo : e.platformHome;
        const sf::Vector2f toTarget = target - e.position;
        const float dist =
            std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
        const float step = e.platformSpeed * dt;
        if (dist <= step)
        {
            e.frameDelta = toTarget; // arrive exactly, then turn around
            e.position = target;
            e.platformSign = -e.platformSign;
        }
        else
        {
            e.frameDelta = toTarget / dist * step;
            e.position += e.frameDelta;
        }
    }
}

void Game::resolveCollisions(Entity& mover, float movement, bool xAxis)
{
    // Collision happens between hitboxes, not sprite boxes: the
    // mover's feet against the other entity's feet (or full box).
    auto pushOut = [&](sf::Vector2f otherPos, sf::Vector2f otherSize) {
        sf::Vector2f boxSize = hitboxSizeOf(mover);
        sf::Vector2f boxPos = mover.position + mover.hitboxOffset;
        if (!math::rectsOverlap(boxPos, boxSize, otherPos, otherSize))
            return;
        if (xAxis)
            mover.position.x = movement > 0.f
                                   ? otherPos.x - mover.hitboxOffset.x - boxSize.x
                                   : otherPos.x + otherSize.x - mover.hitboxOffset.x;
        else
            mover.position.y = movement > 0.f
                                   ? otherPos.y - mover.hitboxOffset.y - boxSize.y
                                   : otherPos.y + otherSize.y - mover.hitboxOffset.y;
    };

    for (const auto& other : m_entities)
    {
        if (&other == &mover || !other.solid)
            continue;
        // One-way (Lesson 57): only a platformer falling onto the
        // top ever collides — the same previous-position question
        // the tile guard asks. Everything else (rising through it,
        // approaching from the side or below, the top-down model,
        // for whom above/below means nothing) passes through.
        if (other.oneWay)
        {
            const sf::Vector2f boxPos = mover.position + mover.hitboxOffset;
            const sf::Vector2f boxSize = hitboxSizeOf(mover);
            const float otherTop = other.position.y + other.hitboxOffset.y;
            if (mover.moveModel != MoveModel::Platformer || xAxis ||
                movement <= 0.f ||
                boxPos.y + boxSize.y - movement > otherTop + 0.01f)
                continue;
        }
        pushOut(other.position + other.hitboxOffset, hitboxSizeOf(other));
    }
    for (const auto& block : m_blocks)
        pushOut(block.getPosition(), block.getSize());

    // Tiles (Lesson 56): a platformer's ground can be the grid
    // itself. Only the tiles the hitbox overlaps are tested — at
    // 20-unit tiles under a 28x22 feet box, a handful — and the
    // same pushOut answers them, so tile ground and entity ground
    // can never disagree about where a surface is. The top-down
    // model skips this pass entirely: its rock is scenery it
    // walks over, and Gold Rush must never find a wall here.
    if (mover.moveModel == MoveModel::Platformer)
    {
        const sf::Vector2f boxSize = hitboxSizeOf(mover);
        sf::Vector2f boxPos = mover.position + mover.hitboxOffset;
        const int tx0 = static_cast<int>(std::floor(boxPos.x / TILE_SIZE));
        const int ty0 = static_cast<int>(std::floor(boxPos.y / TILE_SIZE));
        const int tx1 =
            static_cast<int>(std::floor((boxPos.x + boxSize.x) / TILE_SIZE));
        const int ty1 =
            static_cast<int>(std::floor((boxPos.y + boxSize.y) / TILE_SIZE));
        for (int ty = ty0; ty <= ty1; ++ty)
            for (int tx = tx0; tx <= tx1; ++tx)
            {
                if (!(tx >= 0 && ty >= 0 && tx < m_tilesX && ty < m_tilesY &&
                      tileSolid(tileAt(tx, ty))))
                    continue;
                boxPos = mover.position + mover.hitboxOffset; // pushes move us
                // The previous-position guard: a Y push is only a
                // landing if the hitbox's edge was on the tile's
                // side of its surface LAST frame. Without it, a
                // player standing beside a taller column — his box
                // overlapping its side — gets "landed" on top of it
                // by gravity's micro-move: a step-up teleport.
                if (!xAxis && movement > 0.f &&
                    boxPos.y + boxSize.y - movement > ty * TILE_SIZE + 0.01f)
                    continue;
                if (!xAxis && movement < 0.f &&
                    boxPos.y - movement < (ty + 1) * TILE_SIZE - 0.01f)
                    continue;
                pushOut({tx * TILE_SIZE, ty * TILE_SIZE},
                        {TILE_SIZE, TILE_SIZE});
            }
    }
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

    // Entities are data; their picture is derived from the data each
    // frame — a sprite cut from the sheet when they have one, the
    // colour rectangle when they don't. Two passes so the player
    // always draws on top.
    sf::RectangleShape shape;
    sf::Sprite sprite(m_sheet);
    for (int pass = 0; pass < 2; ++pass)
    {
        for (const auto& entity : m_entities)
        {
            if ((pass == 1) != entity.playerControlled)
                continue;
            // The spawn pop (Lesson 49) scales the player's picture
            // around his feet: same centre-bottom, eased size. Every
            // other entity — and the player once settled — draws at
            // its true position and size.
            sf::Vector2f drawPos = entity.position;
            sf::Vector2f drawSize = entity.size;
            if (entity.playerControlled && !m_spawnPop.done())
            {
                const float s = m_spawnPop.value();
                drawSize = entity.size * s;
                drawPos = {entity.position.x + (entity.size.x - drawSize.x) / 2.f,
                           entity.position.y + (entity.size.y - drawSize.y)};
            }
            if (entity.sprite == sprites::CRATE)
            {
                // File-backed: the crate's texture comes from disk,
                // through the resource cache (Lesson 20). If the file
                // is missing, the cache hands back the placeholder.
                sf::Sprite crateSprite(m_resources.get("assets/crate.tga"));
                crateSprite.setPosition(drawPos);
                crateSprite.setScale({drawSize.x / 32.f, drawSize.y / 32.f});
                target.draw(crateSprite);
                continue;
            }
            if (entity.sprite == sprites::NONE)
            {
                shape.setSize(drawSize);
                shape.setPosition(drawPos);
                shape.setFillColor(entity.color);
                target.draw(shape);
                continue;
            }
            sprite.setTextureRect(
                sf::IntRect({sprites::PLAYER_X + entity.frame * sprites::FRAME, 0},
                            {sprites::FRAME, sprites::FRAME}));
            sf::Vector2f scale(drawSize.x / sprites::FRAME,
                               drawSize.y / sprites::FRAME);
            if (entity.flipX)
            {
                // Mirror in place: the position already carries the
                // +size.x shift and the negative scale does the flip.
                // An origin here would shift the sprite a second time
                // (Lesson 44: with origin FRAME it rendered one full
                // entity width right of its hitbox when facing left).
                sprite.setOrigin({0.f, 0.f});
                sprite.setPosition(drawPos + sf::Vector2f(drawSize.x, 0.f));
                sprite.setScale({-scale.x, scale.y});
            }
            else
            {
                sprite.setOrigin({0.f, 0.f});
                sprite.setPosition(drawPos);
                sprite.setScale(scale);
            }
            target.draw(sprite);
        }
    }

    // Particles draw over the world and its actors, under the
    // debug overlays (Lesson 50).
    if (feel.enabled && feel.particlesEnabled)
        m_particles.draw(target);

    // Shockwave rings (Lesson 52): an expanding outline that fades
    // as it grows — the shape of "something happened here".
    if (feel.enabled && feel.particlesEnabled)
    {
        for (const Ring& ring : m_rings)
        {
            const float progress = 1.f - ring.life / ring.maxLife;
            const float radius =
                3.f + (ring.maxRadius - 3.f) * ease::outCubic(progress);
            sf::CircleShape circle(radius, 36);
            circle.setPosition(ring.position - sf::Vector2f(radius, radius));
            circle.setFillColor(sf::Color::Transparent);
            sf::Color edge = ring.color;
            edge.a = static_cast<std::uint8_t>(200.f * (1.f - progress));
            circle.setOutlineColor(edge);
            circle.setOutlineThickness(2.5f);
            target.draw(circle);
        }
    }

    if (showHitboxes)
    {
        sf::RectangleShape box;
        box.setFillColor(sf::Color::Transparent);
        box.setOutlineColor(sf::Color::Red);
        box.setOutlineThickness(1.f);
        for (const auto& entity : m_entities)
        {
            box.setSize(hitboxSizeOf(entity));
            box.setPosition(entity.position + entity.hitboxOffset);
            target.draw(box);
        }
    }
}

sf::Vector2f Game::playerPosition() const
{
    return m_entities[m_playerEntity].position;
}

sf::Vector2f Game::playerCenter() const
{
    const Entity& player = m_entities[m_playerEntity];
    return player.position + player.size / 2.f;
}

bool Game::saveScene(const std::string& path) const
{
    std::ofstream out(path);
    if (!out)
    {
        Log::error("Could not open scene file for writing: " + path);
        return false;
    }
    auto num = [](float v) {
        std::ostringstream ss;
        ss << std::setprecision(8) << v;
        return ss.str();
    };
    out << "# MyEngine scene\n";
    out << "version=1\n";
    out << "seed=" << m_seed << "\n";
    out << "noiseScale=" << num(m_noiseFrequency) << "\n";
    out << "entities=" << m_entities.size() << "\n";
    for (const auto& e : m_entities)
    {
        // name|x|y|w|h|r|g|b|controlled|solid|sprite — names must not
        // contain '|' (ours never do; the format will grow escaping
        // the day a name needs one).
        out << "entity=" << e.name << '|' << num(e.position.x) << '|'
            << num(e.position.y) << '|' << num(e.size.x) << '|' << num(e.size.y)
            << '|' << static_cast<int>(e.color.r) << '|'
            << static_cast<int>(e.color.g) << '|' << static_cast<int>(e.color.b)
            << '|' << (e.playerControlled ? 1 : 0) << '|' << (e.solid ? 1 : 0)
            << '|' << e.sprite << "\n";
    }
    Log::info("Scene saved to " + path + " (" +
              std::to_string(m_entities.size()) + " entities)");
    return true;
}

bool Game::loadScene(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
    {
        Log::error("Could not open scene file: " + path);
        return false;
    }
    // Parse into temporaries first: a scene loads whole or not at
    // all — never a half-loaded world.
    std::uint32_t seed = m_seed;
    float noiseScale = m_noiseFrequency;
    std::vector<Entity> loaded;
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line))
    {
        ++lineNo;
        if (line.empty() || line[0] == '#')
            continue;
        auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        try
        {
            if (key == "version")
            {
                if (std::stoi(value) != 1)
                {
                    Log::error("Scene version not supported: " + value);
                    return false;
                }
            }
            else if (key == "seed")
                seed = static_cast<std::uint32_t>(std::stoul(value));
            else if (key == "noiseScale")
                noiseScale = std::stof(value);
            else if (key == "entity")
            {
                std::vector<std::string> f;
                std::stringstream ss(value);
                std::string part;
                while (std::getline(ss, part, '|'))
                    f.push_back(part);
                if (f.size() != 11)
                    throw std::runtime_error("bad entity field count");
                Entity e;
                e.name = f[0];
                e.position = {std::stof(f[1]), std::stof(f[2])};
                e.size = {std::stof(f[3]), std::stof(f[4])};
                e.color = sf::Color(static_cast<std::uint8_t>(std::stoi(f[5])),
                                    static_cast<std::uint8_t>(std::stoi(f[6])),
                                    static_cast<std::uint8_t>(std::stoi(f[7])));
                e.playerControlled = f[8] == "1";
                e.solid = f[9] == "1";
                e.sprite = std::stoi(f[10]);
                loaded.push_back(e);
            }
        }
        catch (const std::exception&)
        {
            Log::warning("Scene line " + std::to_string(lineNo) +
                         " skipped (unparseable): " + line);
        }
    }
    if (loaded.empty())
    {
        Log::error("Scene file has no entities: " + path);
        return false;
    }
    m_seed = seed;
    m_noiseFrequency = noiseScale;
    regenerateWorld();
    m_entities = loaded;
    for (auto& e : m_entities)
        applySpriteHitbox(e);
    m_playerEntity = 0;
    for (std::size_t i = 0; i < m_entities.size(); ++i)
        if (m_entities[i].playerControlled)
            m_playerEntity = i;
    Log::info("Scene loaded from " + path + " (" +
              std::to_string(m_entities.size()) + " entities, seed " +
              std::to_string(m_seed) + ")");
    return true;
}
