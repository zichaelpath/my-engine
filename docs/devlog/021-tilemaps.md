# Devlog 021 — Tilemaps (The Field Becomes Data — Arc 4 Closes)

## Goal

Turn the terrain from a baked picture into a tilemap: a grid of tile ids the engine can query, reason about, and — first proof — let affect gameplay. Movement speed now depends on the tile under the player's feet.

## Concept

- **Data first, picture second.** `generateField` used to sample noise and bake coloured triangles in one breath — the terrain existed only as pixels. Now it classifies first: every 20×20 cell becomes a tile id (deep water, water, sand, grass, forest, rock) stored in a `vector<int>`, and the vertex array is built *from the ids*, with the id→colour mapping in exactly one place. The rendered world is pixel-identical to before — which is the point: the refactor changed what the terrain *is* without changing what it *looks like*.
- **A tilemap earns its keep by being queried.** `tileAtWorld(pos)` converts world coordinates to a cell and answers the id (-1 outside). The first customer is movement: the tile under the player's *feet* (the hitbox centre, not the sprite centre — Lesson 19's feet again) scales speed: deep water 0.45×, water 0.55×, sand 0.85×, forest 0.90×, rock 0.70×, grass full speed. Wading feels different from walking the moment you step in, and no other system had to change — the movement code just asks the map a question.
- **What this unlocks, honestly noted:** tiles that can be asked about can be *edited* (the Arc 5 editor will paint them — and painted tiles will need a scene-format extension, since the seed recipe alone won't reproduce edits), collided with selectively (deep water as a wall is now a one-line policy change), and pathed over. None of that was possible while the terrain was a picture.

## Code built

- `tiles` id constants (Game.hpp); `Game` stores `m_tiles` + grid dimensions.
- `generateField` split into classify-then-render; palette function keyed by id.
- Queries: `tileAtWorld`, `playerTile` (feet-based), `tileName` for display.
- `Game::update`: terrain speed factor from the tile under the feet.
- Editor: a "Tile: …" readout in the debug panel, so the query is visible while you walk.

## What broke

Nothing in the build. The *test* broke, instructively: the first "grass" control run travelled only 63 px and the second 131 px, both looking like failures — until the editor's new Tile readout explained them. The chosen "grass plain" start was actually a Water tile at a pond's edge, and the run east crossed into the big eastern lake and waded most of the way (159 px in the editor run, Tile: Water at both ends). The instrumentation caught the experiment's own false assumption — which is rather the argument for building readouts.

## Verification

Built clean on the Linux rig. The rendered world is pixel-identical to Lesson 20's (same thresholds, now via ids). The speed factor was measured with the scene system as a test harness — crafted `scene.txt` starts, a timed 1.0 s drive east, and an F5 save as the measuring tape: from deep inside the western lake, the player covered **108 px** in one second (the 0.45–0.55 wading band, as coded; full speed would be 200). The mixed-terrain editor run (`docs/shots/lesson-21-editor.png`) covered 159 px with the panel's Tile readout tracking the ground truth at both ends, and the readout itself is visible in the shot: "Tile: Water". A pure full-speed grass run stayed unmeasured — the continent's east is lake country, and the devlog says so rather than inventing a cleaner number.

## Explain-it check

_(Post-handover format: the rendered world didn't change at all this lesson — so what did? Why is the speed sampled at the feet instead of the sprite centre? Why will painted tiles force a scene-format change that generated tiles didn't? — the answers live in the Concept section above.)_

## Next step

Arc 5 opens — Lesson 22: selection. Click an entity in the editor's Game View and it becomes *the selected entity* (with a highlight), the foundation the hierarchy panel and inspector will stand on.
