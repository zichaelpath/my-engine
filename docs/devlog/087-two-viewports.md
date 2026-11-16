# Devlog 087 — Two Viewports: One World, Two Eyes

## Goal

Arc 8's last build lesson is the roadmap's flagged stretch goal, attempted for real: a second viewport — a "Game View 2" panel with its own camera, watching the same world from somewhere else while the main view follows the player. An author can park it on the Goal, the door, the far side of the map, and watch both ends of a level at once.

## Concept

- **A viewport is a camera plus a render — nothing more.** This works at all because of a decision made long before it was needed: `Game::render` is `const`. Rendering cannot change the game, so rendering it twice cannot desync anything; the second view is the same state through a different view matrix. Had rendering carried side effects (it very easily could have), multi-viewport would mean untangling them first. Purity, chosen early for testability, pays out here as a feature.
- **The second camera belongs to the author.** The main camera follows the player, shakes on impacts, and serves the game feel; camera2 does none of that — no follow, no shake — because its job is surveillance, not play. It pans (right-drag) and zooms (wheel) with the same formulas as the main view, clamps to the world, and starts on the world's centre. Deliberately *not* shared state: two views that mirror each other's camera would be one view with extra steps.
- **Agreement where it matters.** The selection highlight is drawn into both textures, so "the entity I selected" is findable in whichever view you're looking at. Picking, dragging and painting stay with the main view only — one editing hand, two eyes; two editing hands would need an answer for which view's drag wins mid-gesture, and that answer isn't worth its complexity budget yet.
- **Off means free.** The second render happens only while the panel is enabled (a checkbox beside Camera follow); a closed viewport costs nothing per frame. Viewports are a sometimes-tool, and the frame budget shouldn't pay rent for them.

## Code built

- `src/main.cpp` — `camera2` + `zoom2` + the Game View 2 enable checkbox; the second RenderTexture and its render pass (own view, clamp, selection overlay, no shake/follow); the Game View 2 panel with its own right-drag pan and wheel zoom.

## Verification

- Measured, not eyeballed: a probe renders one Game through camera A twice and camera B once. Same-camera difference: **0** (render is pure, pixel for pixel). Two-camera difference: **6.58M** across sampled channels — a genuinely different place. The screenshot (`docs/shots/lesson-87-two-viewports.png`) is those two renders side by side: the spawn area with its crate on the left, the far lakes on the right — one world, one frame, two eyes.
- Harness: **63 passed, 0 failed** (no Game changes this lesson). Editor smoke: clean boot with the viewport code in; enabling the panel and dragging its camera are GUI paths for the playtest list, as with the other Arc 8 panels.

## Explain-it check

_(Post-handover format: what property of Game::render makes a second viewport safe, and when was that property actually decided? Why does camera2 get no follow and no shake? Why does selection draw in both views while editing lives in only one? — the answers live in the Concept section above.)_

## Next step

Lesson 88: Arc 8 closes — the editor's growth gets its regression pass, and MyEngine ships 1.7 with both games repackaged.
