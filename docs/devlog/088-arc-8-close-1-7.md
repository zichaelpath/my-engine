# Devlog 088 — Arc 8 Close: MyEngine 1.7

## Goal

Close Arc 8 — The Editor Grows Up, Windows and All — the way the previous arcs closed: a capstone check that plays the arc's promise end to end, a version bump, both games repackaged and clean-room verified, and the record written.

## What the arc built (Lessons 83–87)

- **83 Terrain painting** — the author's hand on the generated field: differences-only tile overrides in the scene file, one undo timeline for entities and paint.
- **84 The pop-out Game View** — the world in its own OS window; one render path, input unified through `ViewPointer`, the layout persisted in engine.ini.
- **85 The Levels panel** — the campaign's geography edited in the editor and written through to the project card on every change.
- **86 Play-test travel** — editor tests follow doors and Goals across the campaign, with a scene-aware snapshot restoring the start exactly.
- **87 Two viewports** — a second camera on the same world, safe because `Game::render` is const; the roadmap's stretch goal, landed.

## The capstone (check 64)

The arc's deepest promise is that the author's hand never corrupts the machine's world: paint is a layer, not a mutation. The capstone plays it whole — a project is created, its scene painted in a fresh Game, saved; then, cold, the project and its start scene are loaded the way the Runtime meets them and the paint is there; then the overrides are cleared and the tile grid's FNV-1a fingerprint equals the **1.0 baseline exactly**. Six arcs after the fingerprint was pinned, generation is still bit-identical under everything the editor learned to do.

## Verification

- Harness: **64 passed, 0 failed.**
- Clean room: both 1.7.0 packages extracted to a bare directory and booted — `MyEngine 1.7.0 - Runtime`, Gold Rush loads its project, Temple Run loads and builds its canonical seed-20261 ascent. The screenshot (`docs/shots/lesson-88-arc8-close.png`) is the packaged Gold Rush's title screen from that run.
- The arc's GUI paths (paint strokes, the pop window's mouse, panel buttons, the second camera's drag) remain on the playtest gate's hands-on list — their machinery is harness- and probe-verified; their clicks are Zac's to confirm, as recorded honestly in each lesson.

## Explain-it check

_(Post-handover format: state the arc in one sentence per lesson — what did the editor learn to do in 83, 84, 85, 86 and 87, and what single property made 87 nearly free? — the devlogs' Concept sections hold the answers.)_

## Next step

Arc 9 — 3D Second Season: the GL host from Phase 1 grows into the engine's 3D path (Lessons ~89–94). After it, Arc 10 ships 2.0 — and the public-repo decision is Zac's to make before that arc opens.
