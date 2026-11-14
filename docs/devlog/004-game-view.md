# Devlog 004 — The Game View (RenderTexture)

## Goal

Stop drawing the game straight onto the window. Draw it into an off-screen `sf::RenderTexture` instead, then show that texture inside its own ImGui panel — the game becomes a picture inside the editor, with panels around it. The app window grows to 1280×720; the game world stays 800×600.

## Concept

A RenderTexture is a **canvas that isn't a screen**. It has the same verbs as the window — `clear`, `draw`, `display` — but `display()` there means "finish drawing into the texture," not "show it." Nothing reaches the real screen until something draws the texture, and in an editor that something is a panel: `ImGui::Image` displays the texture like a photograph of the game.

This is the editor trick everything else builds on. Once the game is just an image inside a panel, panels can sit beside it, it can be resized, paused, or shown twice — the world no longer owns the window. It also moves the meaning of the coordinates: the movement clamps (0–750, 0–550) now describe the edges of the *game view*, not the window.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- Window resized to 1280×720.
- `sf::RenderTexture gameView` (800×600) created once in setup, before the loop.
- Draw restructured into two stages: (1) clear/draw/display into `gameView`; (2) declare a "Game View" panel with `ImGui::Image(gameView.getTexture(), ...)`, clear the real window to dark grey, `ImGui::SFML::Render`, `display`.
- Debug panel unchanged — its clear-colour picker now recolours the game view's background, not the whole window.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — game visible inside its panel, player moves and clamps inside the view, speed slider and colour picker still work, panels draggable)_

## Explain-it check

_(yours to fill in)_

## Next step

Lesson 5 — to be decided from here (panel layout/docking, or play/pause control of the game view).
