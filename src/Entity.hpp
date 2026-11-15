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
};
