# Devlog 007 — The Resizable Game View

## Goal

Make the game view fill its panel at whatever size the panel happens to be: drag the dock layout or resize the window, and the RenderTexture is recreated to match, so the view is always exactly panel-sized — never stretched, never letterboxed.

## Concept

The panel and the renderer take turns, one frame apart. The Game View panel **measures** itself (`ImGui::GetContentRegionAvail`) and stores the result in a variable that persists between frames. Before the next frame's draw into the texture, the renderer **applies** that size: if it differs from the texture's current size, the texture is recreated with `resize(...)` and its view (camera) is reset to cover the new canvas exactly. One frame of lag between dragging and the texture catching up — real, and invisible.

Two guards keep this stable:

- **No feedback loop.** The image is displayed at *exactly* the measured content size — never bigger. If the image overflowed the panel, scrollbars would appear, the content region would shrink, the texture would shrink, the scrollbars would vanish, and the layout would oscillate forever. Exact fit + "only recreate when the size actually changed" breaks the cycle.
- **No degenerate sizes.** A collapsed panel can report zero or negative space, and a tiny panel makes the movement clamp's max smaller than its min — `std::clamp` with min > max is undefined behaviour. Sizes are checked before use, and the clamp's upper bound is floored at 0 with `std::max`.

Recreating a texture isn't free — during an active drag it happens every frame. Acceptable here; professional editors debounce it (resize only when the drag settles). A known optimisation, deliberately not built yet.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- Setup: `sf::Vector2f panelSize{800.f, 600.f};` — the size the Game View panel last reported; persists across frames.
- Game View panel: `ImGuiStyleVar_WindowPadding` pushed to (0,0) so the view sits flush in the panel; panel measures `GetContentRegionAvail()` into `panelSize` (when positive), then `ImGui::Image(gameView, panelSize)`.
- Before drawing into the texture: if the wanted size differs from `gameView.getSize()`, `gameView.resize(wantSize)` and `gameView.setView(...)` with a view covering the new size.
- Movement clamps now read the live texture size (`gameView.getSize()`) instead of the hardcoded 750/550, with the upper bound floored via `std::max(0.f, ...)`.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — view follows panel/window resizes with no stretching, growing the view grows the playfield, shrinking it pulls the player back inside via the clamp, no scrollbar flicker/oscillation, pause/step still work)_

## Explain-it check

_(yours to fill in)_

## Next step

Lesson 8 — the split: pull the game out of main.cpp into its own unit that doesn't know where it draws. That's the step that eventually makes the standalone Runtime executable possible.
