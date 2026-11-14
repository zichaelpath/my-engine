# Devlog 003 — First Internal Window (Dear ImGui Debug Panel)

## Goal

Put a window *inside* the engine window: a floating "Engine Debug" panel built with Dear ImGui (via ImGui-SFML) that shows live FPS/frame time and the player's position, and can change the clear colour and the player's speed while the game is running.

## Concept

ImGui is **immediate-mode**: there are no widget objects you create once and update later. Every frame, your code re-declares the whole UI — `Begin(...)`, some widgets, `End()` — and ImGui works out what to draw and what got clicked. The *variables are the state*: `ColorEdit3` writes directly into your `float[3]`, `SliderFloat` writes directly into your `speed`. Nothing to sync, nothing to wire up afterwards.

That's why two things from Lesson 2 have to change shape: `speed` can't be `constexpr` anymore (a slider has to be able to write to it), and the clear colour becomes a `float[3]` array (that's the format ImGui's colour picker edits), converted to `sf::Color` each frame.

## Code built

- CMake: Dear ImGui v1.91.8 + ImGui-SFML v3.0 fetched and linked (`ImGui-SFML::ImGui-SFML`) — already in the repo's CMakeLists before this lesson started.
- Wiring in main.cpp: `ImGui::SFML::Init` after window creation, `ProcessEvent` first inside the event loop, `Update(window, sf::seconds(dt))` after the update section, the panel declared between update and draw, `ImGui::SFML::Render(window)` after `window.draw(player)` and before `display()`, `Shutdown()` after the loop.
- The "Engine Debug" panel: FPS and frame-time text, player position text, `ColorEdit3` on the clear colour, `SliderFloat("Speed", &speed, 50, 600)`.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — panel visible, slider changes speed live, colour picker changes the background live, player still moves, Esc/X still close)_

## Explain-it check

_(yours to fill in)_

## Next step

Lesson 4 — RenderTexture: the game view rendered inside a panel, on the road to a real editor viewport.
