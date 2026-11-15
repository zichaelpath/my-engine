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
}

void Game::generateField()
{
    constexpr float tileSize = 20.f;
    const int tilesX = static_cast<int>(m_worldSize.x / tileSize);
    const int tilesY = static_cast<int>(m_worldSize.y / tileSize);

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
    Log::info("Decor generated: " + std::to_string(m_dots.size()) + " dots");
}

void Game::update(float dt)
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

    sf::Vector2f move = dir * speed * dt;

    // Axis-separated collision: move and resolve one axis at a time.
    // A blocked axis never stops the other, so walls slide instead of
    // sticking. At this engine's speeds (a few pixels per frame) a
    // discrete overlap test cannot tunnel; fast projectiles will one
    // day need swept tests, and will say so when they arrive.
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
        pushOut(other.position + other.hitboxOffset, hitboxSizeOf(other));
    }
    for (const auto& block : m_blocks)
        pushOut(block.getPosition(), block.getSize());
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
            if (entity.sprite == sprites::NONE)
            {
                shape.setSize(entity.size);
                shape.setPosition(entity.position);
                shape.setFillColor(entity.color);
                target.draw(shape);
                continue;
            }
            int baseX = entity.sprite == sprites::PLAYER ? sprites::PLAYER_X
                                                         : sprites::CRATE_X;
            sprite.setTextureRect(
                sf::IntRect({baseX + entity.frame * sprites::FRAME, 0},
                            {sprites::FRAME, sprites::FRAME}));
            sf::Vector2f scale(entity.size.x / sprites::FRAME,
                               entity.size.y / sprites::FRAME);
            if (entity.flipX)
            {
                sprite.setOrigin({static_cast<float>(sprites::FRAME), 0.f});
                sprite.setPosition(entity.position +
                                   sf::Vector2f(entity.size.x, 0.f));
                sprite.setScale({-scale.x, scale.y});
            }
            else
            {
                sprite.setOrigin({0.f, 0.f});
                sprite.setPosition(entity.position);
                sprite.setScale(scale);
            }
            target.draw(sprite);
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
