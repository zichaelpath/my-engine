#pragma once

#include <SFML/Graphics.hpp>

#include <string>

// An Entity is a thing in the world, described by data alone: where it
// is, how big, what colour, and whether the player drives it. No
// behaviour lives here yet — systems (movement today, collision and
// AI later) operate ON entities from outside. That separation is the
// seed of the component design Arc 3 grows toward: today the fields
// sit in one struct; when entities start needing different subsets
// (a crate has no input, a ghost has no collision), the struct splits.
// Who owns an entity's vertical motion (Lesson 54). TopDown: the
// player steers both axes directly (Gold Rush). Platformer:
// gravity owns Y, input owns X, and the ground is a fact the
// entity carries. One collision system serves both.
enum class MoveModel { TopDown, Platformer };

struct Entity
{
    std::string name;
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f size{50.f, 50.f};
    sf::Color color = sf::Color::White;
    bool playerControlled = false;
    bool solid = true; // solid entities block movement (Lesson 15)

    // Appearance (Lesson 17): which sprite-sheet character this is
    // (sprites::NONE draws the colour rectangle instead), the current
    // animation frame, the clock that advances it, and facing.
    int sprite = -1;
    int frame = 0;
    float animTime = 0.f;
    bool flipX = false;

    // Collision (Lesson 19): the hitbox is a rectangle inside the
    // entity's sprite box — an offset and a size, relative to the
    // entity's position. A zero size means "the whole entity". The
    // player uses a small feet box: the sprite's transparent margins
    // and the head/shoulders no longer collide, only the feet do.
    sf::Vector2f hitboxOffset{0.f, 0.f};
    sf::Vector2f hitboxSize{0.f, 0.f};

    // Platformer state (Lesson 54): the movement model, the
    // velocity gravity integrates, and whether the last frame
    // ended standing on something. TopDown entities ignore all
    // three; they exist so a game can mix models in one world.
    MoveModel moveModel = MoveModel::TopDown;
    sf::Vector2f velocity{0.f, 0.f};
    bool grounded = false;

    // Platforms (Lesson 57): a platform patrols between its
    // position and platformTo at platformSpeed (0 = stands still),
    // carries a grounded platformer rider by its frameDelta, and —
    // if oneWay — only collides from above: you jump up through
    // it and land on it. The patrol DEFINITION (to/speed/oneWay)
    // persists in scene format v2 (Lesson 69); the runtime state
    // below it (home/sign/frameDelta) does not.
    bool platform = false;
    bool oneWay = false;
    sf::Vector2f platformTo{0.f, 0.f};
    float platformSpeed = 0.f;
    sf::Vector2f platformHome{0.f, 0.f}; // captured on first motion
    bool platformInit = false;
    int platformSign = 1;                // +1 toward platformTo
    sf::Vector2f frameDelta{0.f, 0.f};   // this frame's travel

    // Type & script (Lesson 68): the entity's type name ("" =
    // untyped) and the project-relative path of its Lua script
    // ("" = none). The type is the data half of the scripting
    // model: a definition in the game's registry hands the entity
    // its defaults; the script is the behaviour half.
    std::string type;
    std::string script;
};

// A type definition (Lesson 68): the defaults a new entity of
// this type is stamped with. Types live in the Game's registry,
// defined in code or loaded from a types file; a scene stores
// only the type NAME on each entity, so a game's cast can be
// rebalanced in one place.
struct EntityType
{
    std::string name;
    sf::Vector2f size{50.f, 50.f};
    sf::Color color = sf::Color::White;
    bool solid = true;
    MoveModel moveModel = MoveModel::TopDown;
    int sprite = -1;
    std::string script;
};
