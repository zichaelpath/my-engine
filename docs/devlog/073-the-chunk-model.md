# Devlog 073 — The Chunk Model: Arc 6 Opens

## Goal

Open the second game — the vertical Temple Run — the way the roadmap ordered: prototype the **chunk model**. Chunks are authored vertical slices of the climb, stored as data in the game's project; a seeded generator *deals* them into an ascent. No endless machinery yet, no rising pressure — just the vocabulary, the deal, and a player standing at the foot of a dealt climb.

## Concept

- **Authored chunks, dealt like cards.** The generator invents nothing geometric: every platform a player will ever stand on was placed by a designer in a chunk definition (`games/temple-run/chunks.txt`, five chunks). What the seed decides is the *order*. This is the Temple Run answer from the roadmap — procgen as a dealer of human-made material, not a replacement for it — and it keeps the engine's determinism law: same seed, same deal, verified entity-for-entity.
- **Bookends make any deal climbable.** Every chunk opens on the same entry landing (centre, local y=165) and leaves from the same exit landing (centre, y=25). Any chunk can sit on any other: the exit of the lower and the entry of the upper are always 60 units apart at the same x. The variety lives in the middle; the transitions are guaranteed by construction, not by testing every pairing. Inside a chunk, every hop respects the measured jump (rise ≈ 102, range ≈ 145): no step up over 55, no edge gap over 130.
- **Chunk-local coordinates.** A platform is written once, in the chunk's own frame (x across the 800-wide shaft, y down from the chunk's top, patrol targets local too). `buildAscent` stacks the dealt chunks from the floor upward — each chunk's own height counts, so chunks of different heights stack exactly — and translates everything into world space in one place.
- **A second game replaces the stage, honestly.** `buildAscent` clears Gold Rush's entities, dots, and terrain blocks, resets the run state, and *carves* its shaft: rock tiles are solid to a platformer and a dealt chunk cannot know the mountain's geology, so the shaft's column of the tile field is flattened to grass. The base world's determinism is untouched (the pinned checksum never builds an ascent), and a `newRun` restores Gold Rush whole.
- **The deal has manners.** A private `Random` keyed by the run seed alone deals the chunks, so the climb depends on the seed you pass, not on whatever world the Game happened to be showing. A chunk never repeats back-to-back — a deterministic nudge to the next definition, not a re-roll loop, so the deal stays a pure function of the seed.

## Code built

- `src/Entity.hpp` — `ChunkPlatform` / `ChunkDef`: the chunk as pure data.
- `src/Game.hpp` / `src/Game.cpp` — `loadChunks` (the types file's grammar grown a level: `chunk=` / `height=` / `platform=x|y|w|oneway[|toX|toY|speed]`, bad lines warn and skip) and `buildAscent(seed, count)` (stage replacement, the deal, the stacking, the carve, the floor, the platformer player).
- `games/temple-run/` — the second game's own project folder (Arc 4's layout): `project.myengine` (seed 20261), `chunks.txt` with five authored chunks (Steps, Zigzag, Elevator, Islands, Gauntlet), empty `assets/` / `scenes/` / `scripts/`.
- `tests/tests_main.cpp` — the chunk-model check (suite now 52).

## Verification

- Harness: **52 passed, 0 failed** — chunk files parse exactly (patrol fields, a 260-tall chunk among 200s); a four-chunk deal never repeats adjacently; Chunk0P0 and Chunk1P0 land on hand-computed world coordinates through *cumulative variable heights*; a forced one-chunk deal maps a patrol's world target to the exact local delta (+200, −60); the player stands grounded on the floor as a platformer; two Games dealt seed 42 produce identical layouts, seed 7 a different one.
- Probe (the shipped chunks, the project's own seed): deal **Gauntlet, Steps, Islands, Steps, Zigzag** — top at y=150, 26 entities — and the player, driven through the input seam, jumps from the floor (y=1100) and lands grounded at y=1005: one jump cleared the entry landing and Gauntlet's second step (95 units up).
- Screenshot: `docs/shots/lesson-73-chunk-model.png` — a fixed camera at the climb's foot: on the floor, then standing on the dealt Gauntlet's second landing.
- Two slips, owned: the harness's first run failed on *my check's* bug — it captured the "before" layout after 30 updates had already moved a patrol platform, then blamed the deal; the capture now happens before the world ticks. And the probe's first screenshot framed empty terrain — `Game::render` draws in the target's existing view, and the default view never sees y=1100; the probe now aims a fixed view at the shaft, which is also the honest way to show a rise.

## Explain-it check

_(Post-handover format: why do all chunks share the same entry and exit landings instead of letting each chunk start and end where it likes? Why does the deal use a private Random instead of the Game's own streams? Why does buildAscent flatten the shaft's tiles instead of scanning for a naturally clear column, the way the climber test did? — the answers live in the Concept section above.)_

## Next step

Lesson 74: the rising pressure — the doom that climbs behind the player, the height score it races against, and the run's end when it catches him.
