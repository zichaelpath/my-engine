#pragma once

#include <SFML/Graphics.hpp>

// The engine's built-in sprite sheet, drawn in code at startup — no
// image files. One row of 32x32 cells: four walk-cycle frames of the
// player character, then the crate. Arc 4's resource system will add
// file loading; the atlas idea (one texture, many rectangles) is the
// same either way, which is why it is worth learning on a sheet we
// can generate deterministically.
namespace sprites
{
constexpr int FRAME = 32;      // every cell is FRAME x FRAME pixels
constexpr int PLAYER_X = 0;    // player frames start at x=0 (4 frames)
constexpr int CRATE_X = 128;   // the crate's single cell
constexpr int SHEET_W = 160;
constexpr int SHEET_H = 32;

// Sprite ids stored on entities.
constexpr int NONE = -1;
constexpr int PLAYER = 0;
constexpr int CRATE = 1;
} // namespace sprites

sf::Image buildSpriteSheetImage();
