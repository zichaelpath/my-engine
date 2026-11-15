# Devlog 010 — The Procgen Foundation (Master Seed + Named Streams)

## Goal

Give the engine its procedural-generation bedrock: one master seed, and **named RNG streams** derived from it, living in the game library. Then prove it visibly: a scatter of terrain blocks and decor dots generated from the seed — same seed, same layout, in the editor and the runtime, forever.

## Concept

A random number generator is a machine that produces a fixed, endless sequence of numbers from a starting value — the **seed**. Same seed, same sequence, guaranteed. That guarantee is the whole foundation: a world generated from seed 12345 can be rebuilt exactly, by anyone, anywhere, which is what makes procgen worlds shareable, testable, and saveable (a save file can store one number instead of a world).

**Named streams** are the design decision that makes this scale. Instead of one global RNG that everything draws from — where adding a single random draw anywhere reshuffles every system downstream of it — each system gets its own stream ("terrain", "decor", later "enemies", "loot"), seeded independently from the master seed plus the stream's name. Terrain's layout then depends only on the seed and terrain's own draws. Reshuffling decor cannot move a single block.

The machinery is chosen for determinism, not convenience:

- `std::mt19937` — its sequence is **specified by the C++ standard**, identical on every conforming implementation. (`rand()` is not.)
- Stream seeds are derived via `std::seed_seq` from {master seed, name hash} — also standard-specified.
- The name hash is a hand-written **FNV-1a** — six lines — because `std::hash<std::string>` is explicitly unspecified and could differ between compilers. Determinism is never rented from unspecified behaviour.
- Honest footnote: the standard guarantees the *engine's* sequence, not the exact mapping of `std::uniform_*_distribution`s across different standard libraries. On one toolchain (our editor + runtime) the guarantee is airtight; cross-compiler bit-exactness would need hand-rolled distributions, noted for the future.
- `std::random_device` appears exactly once, in the editor, to *pick* new master seeds. It is a seed-picker, never a generator — anything it touches directly could never be reproduced.

Generation is a function of the **seed alone** — never of window size, panel size, or frame count. Blocks and dots are generated in the game's fixed logical space (800×600), whatever any host's canvas happens to be. Anything else leaking in would break the same-seed-same-world promise.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- New `src/Random.hpp` / `src/Random.cpp` (in GameLib): `class Random` — master seed, `reseed`, lazily-created named streams in a map, `rangeInt` / `rangeFloat` / `chance` draws addressed by stream name; FNV-1a name hash; streams seeded via `seed_seq` + `mt19937`.
- `Game`: owns a `Random` and a master seed (default 12345); `setSeed` reseeds and regenerates; `generateTerrain()` places 12 slate blocks from the "terrain" stream; `generateDecor()` places 20 gold dots from the "decor" stream; `render` draws blocks and dots under the player.
- Editor: new **Procgen** panel — current seed, seed input + Apply, Random seed button, Reshuffle terrain / Reshuffle decor buttons.
- `CMakeLists.txt`: `src/Random.cpp` added to the `GameLib` target.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — relaunch gives the identical layout; Reshuffle decor moves only the dots; Reshuffle terrain moves only the blocks; typing 12345 restores the original layout exactly; the runtime shows the same default-seed layout as the editor)_

## Explain-it check

_(yours to fill in)_

## Next step

Lesson 11 — noise: value noise built on these streams, shown as a tile grid in the game view — the raw material of terrain.
