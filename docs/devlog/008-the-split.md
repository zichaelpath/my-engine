# Devlog 008 — The Split (Game Leaves main.cpp)

## Goal

Move the game — player, movement, game clock, background colour — out of `main.cpp` into its own unit, `Game.hpp` / `Game.cpp`. On screen, nothing changes at all. In structure, everything changes: `main.cpp` becomes the host (window, editor, panels), and `Game` becomes a thing the host *runs* — a unit that updates itself and renders into any `sf::RenderTarget` it's handed, knowing nothing about ImGui, panels, windows, or textures.

## Concept

A class split across two files. The **header** (`.hpp`) is the promise: what the class is called, what you may ask it to do, what state it exposes. The **source** (`.cpp`) is the machinery behind the promise. `#pragma once` at the top of the header guarantees it is only ever included once per build, no matter how many files ask for it.

The host↔game contract:

- The host decides **when time flows**. `playing` and `stepClicked` stay in `main.cpp`; only when the gate opens does the host call `game.update(dt, worldSize)`. Lesson 5's two clocks are now two *files*: the editor owns the decision, the game owns the elapsed time (`game.time` only advances inside `update`).
- The game is **told** how big its world is on each update, rather than owning a window or texture it could measure itself. A parameter is a question the host answers fresh every call.
- The game **renders into whatever it's handed**: `render(sf::RenderTarget&)` works identically for a RenderTexture today and a plain window tomorrow. That ignorance is the entire point — it's what will let the same `Game.cpp` run inside the editor *and* in a standalone Runtime executable.
- The editor's dials are **public data**: `speed`, `clearColor`, `time` sit in the open so the panels can read/write them directly. The player shape is `private` (`m_player`, the `m_` prefix marking "member, hands off") and the host may only *look* at it through `playerPosition()`.
- Another build lesson hiding inside this one: adding a `.cpp` file is a CMake change — a source file that isn't listed in the target doesn't exist as far as the build is concerned.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- New `src/Game.hpp`: `class Game` — public `update(float dt, sf::Vector2f worldSize)`, `render(sf::RenderTarget& target) const`, `playerPosition() const`; public state `speed`, `clearColor[3]`, `time`; private `sf::RectangleShape m_player`. Guarded by `#pragma once`.
- New `src/Game.cpp`: constructor sets up the player (50×50, green, spawn 375,275); `update` advances `time`, polls WASD/arrows, normalises, clamps to the passed-in world size; `render` converts `clearColor` to `sf::Color`, clears the target, draws the player.
- `src/main.cpp`: player/speed/clearColor/gameTime locals deleted; `Game game;` in setup; Debug panel reads/writes the game's public state; the sim gate calls `game.update(dt, ...)`; the draw stage calls `game.render(gameView)`. No game logic remains in main.
- `CMakeLists.txt`: `src/Game.cpp` added to the `MyEngine` target.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — behaviour identical to Lesson 7 in every respect: movement, pause/step, sliders, resizable view; plus the separation test: the string "ImGui" appears nowhere in Game.hpp/Game.cpp)_

## Explain-it check

_(yours to fill in)_

## Next step

Lesson 9 — the Runtime: a second executable target that runs this same Game in a plain window with zero ImGui. One game, two hosts — the first true Editor/Runtime split.
