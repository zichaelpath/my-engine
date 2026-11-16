# Devlog 083 — Terrain Painting: The Author's Hand on the Field

## Goal

Open Arc 8 (The Editor Grows Up): the generated field has been read-only since Lesson 6 — beautiful, deterministic, and untouchable. Give the author a brush: paint tile types in the editor, stored in the scene as overrides over the generation, so a level can be *shaped* without giving up the procedural foundation.

## Concept

- **Paint is an override, never a rewrite.** A painted tile lives in a map — `(tx, ty) → id` — holding *only real differences* from what generation produces at that spot. `generateField` builds the field from the noise, then applies the map over it, every time. Painting a tile back to its generated id *removes* its entry; erasing does the same by hand. A scene with no paint has an empty map and a file identical to yesterday's — the feature's weight is exactly proportional to its use.
- **One pure function makes it all answerable.** Painting needs to know "what would generation put here?" for a single tile. That was locked inside `generateField`'s loop until this lesson hoisted it into `generatedTileAt(tx, ty)` — per-tile pure (one noise sample, one classification, no neighbours), so the field builder, the painter, the eraser and the override filter all ask the same question and get the same answer. The 1.0 grid checksum still matches after the refactor: the field is bit-identical, only its construction is shared now.
- **The format grows additively, again.** Scene files gain `tile=tx|ty|id` lines — written only for overrides, parsed into a temporary map and applied after the world regenerates (so the scene's own seed governs what "generated" means). The version stays 2: an older build reads the file and skips the paint lines as unparseable, losing decoration, never the level. That is the third time the additive discipline (project keys, entity fields, now tile lines) has let a format grow without a migration.
- **One undo timeline for both kinds of edit.** The editor's undo snapshots grew from entity lists to `EditSnapshot`s — entities *plus* the tile-override map — captured and restored together. The alternative (a separate paint-undo stack) would make Ctrl+Z depend on which kind of edit you did last in a way no user can hold in their head; a stroke pushes one snapshot at the grab, exactly like an entity drag, and drags, inspector edits and paint strokes now interleave on one history.
- **The tool is a mode, not a panel trick.** The Terrain panel carries the palette (the six tile types by their engine names, plus the eraser), a 1×1/3×3 brush, a live count of painted tiles, and Clear-all. Paint *mode* is a toggle: while it's on, the Game View's left button paints strokes instead of picking entities — the two verbs never fight over one button, and the mode is visible before it's surprising.

## Code built

- `src/Game.hpp` / `src/Game.cpp` — `generatedTileAt` (hoisted, pure), `setTile` / `eraseTileOverride` / `setTileOverrides` / `tileOverrides`, the override application inside `generateField`, the extracted `rebuildFieldVertices`, overrides cleared by `regenerateWorld` (a fresh world is unpainted), scene save/load of `tile=` lines, and `Game::TILE_SIZE` promoted to the class as the one source.
- `src/main.cpp` (the editor) — the unified `EditSnapshot` undo/redo, the Terrain panel (mode, palette, eraser, brush, count, clear-all), and paint-stroke handling in the Game View (snapshot at the grab, tiles applied while the button is down, picking suppressed in paint mode).
- `tests/tests_main.cpp` — the painting check (suite now 61).

## Verification

- Harness: **61 passed, 0 failed** — a tile starts natural; painting records exactly one override; painting back to the generated id erases it; the eraser restores and then refuses; out-of-bounds and bad-id paints are refused; three painted tiles round-trip through a real scene file with ids and count intact. The determinism check still passes against the 1.0 baseline, proving the `generatedTileAt` refactor changed no generated value.
- Probe: a 10×6 rock plaza with a sand path painted through the public API renders as the after panel of `docs/shots/lesson-83-terrain-painting.png` — and reports **59** overrides for 60 painted tiles: one plaza tile was already rock by generation, so it correctly recorded nothing. The differences-only rule, caught working in the wild.
- Editor smoke: the editor runs with the new panel registered in its layout (imgui.ini records the Terrain window); the panel's wiring is editor-side ImGui over the harness-verified Game API. Click-painting itself joins the playtest gate's hands-on list, like every GUI path.

## Explain-it check

_(Post-handover format: why store differences instead of the whole tile grid in the scene file? Why did undo have to become a combined snapshot instead of a second stack for paint? Why does painting a tile to its generated colour delete data instead of storing it? — the answers live in the Concept section above.)_

## Next step

Lesson 84: the Game View pops out — the editor's world in its own OS window, undockable for a second monitor.
