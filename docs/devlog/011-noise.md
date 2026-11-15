# Devlog 011 — Noise (The Raw Material of Terrain)

## Goal

Build value noise on top of Lesson 10's seed discipline and show it as a coloured tile field in the game view — smooth, blobby, terrain-like variation instead of static. Same seed, same map, in both hosts.

## Concept

Raw RNG output is **white noise**: every value unrelated to its neighbours. Terrain needs the opposite — nearby places should be *similar*, distant places different. **Value noise** gets there in three moves:

1. **Lattice**: imagine an infinite grid of points, one unit apart. Every grid point has a fixed pseudo-random value in [0, 1) — not stored anywhere, but *computed on demand* by hashing (seed, x, y). Any point, any order, any machine: the same value, forever. No grid array, no size limit, no generation order.
2. **Interpolation**: a sample between grid points blends the four surrounding lattice values, weighted by a smoothstep curve (`t²(3−2t)`) so the blend has no hard seams. Nearby samples share lattice corners, so nearby samples are similar — smoothness comes from sharing.
3. **Octaves (fBm)**: one octave gives smooth blobs. Layering copies at double the frequency and half the strength — continents, then hills, then detail — sums to something that reads as terrain. The weights are normalised so the result stays in [0, 1).

The noise is seeded from the master seed through `Random::derivedSeed("noise")` — a derivation that consumes no draws from any stream, so asking for it can never reshuffle anything (Lesson 10's hidden-input discipline, applied to our own API). The field is drawn as **one `sf::VertexArray`** — 40×30 tiles × 2 triangles — instead of 1,200 separate shape draws: the first time the engine batches geometry into a single draw call, and the seed of the future tilemap system.

Heights become colours through a palette: deep water → water → sand → grass → forest → rock. The mapping is arbitrary and will change per game; the heightfield underneath is the engine's product.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- New `src/Noise.hpp` / `src/Noise.cpp` (in GameLib): `class ValueNoise` — `lattice(x, y)` via a 32-bit integer mix of (seed, x, y); `sample(x, y)` bilinear smoothstep interpolation; `fbm(x, y, octaves)`.
- `Random`: new `derivedSeed(name) const` — a master-seed-derived seed for subsystems, consuming no stream draws.
- `Game`: owns a `ValueNoise` (rebuilt in `setSeed` from `derivedSeed("noise")`); `generateField()` builds the tile `VertexArray` (40×30 tiles of 20px over the 800×600 logical world, 4 octaves, palette by height); field drawn under blocks, dots, and player. `setNoiseFrequency` regenerates the field — frequency is a generation parameter, part of the recipe.
- Editor: Procgen panel gains a **Noise scale** slider wired to `setNoiseFrequency`.
- `CMakeLists.txt`: `src/Noise.cpp` added to `GameLib`.

## What broke

- **IntelliSense vs `std::lerp`** (on Zac's laptop): VS Code's checker reported `namespace "std" has no member "lerp"` for a legal C++20 call — IntelliSense was parsing the brand-new file with a stale standard setting. The fix taken anyway: the interpolation is now written out by hand (`a + (b - a) * t`), because the core operation of the lesson shouldn't be rented from a one-line library function.
- **A latent Lesson 10 bug, caught in review**: `Game.hpp` declared `m_random` *before* `m_seed`, but `Random`'s constructor reads `m_seed` — and members initialise in declaration order, so the generator was being seeded from an uninitialised member. (It looked stable because the memory happened to hold the same value each run.) `m_seed` now sits above `m_random`, with a comment recording why. Same review fixed the render order: the player was drawn *under* the blocks and dots; it now draws last.
- **The handover merge**: Zac had pushed his own `derivedSeed` from his laptop (identical implementation) just before handing the project over. Git auto-merged both copies, producing a duplicate definition; deduped in a follow-up commit and rebuilt clean.

## Verification

Built with GCC 13 / CMake on Linux (the new build rig: Dietrich's VM, Xvfb display, SFML audio/network modules off) — zero errors. Both hosts run:

- **Runtime** (`docs/shots/lesson-11-runtime.png`): the full 800×600 world — blue water with sand shores, grass, dark forest, grey rock tiles, slate blocks, gold dots, and the player square on top.
- **Editor** (`docs/shots/lesson-11-editor.png`): Engine Debug at ~60 FPS, Procgen panel showing the Noise scale slider at 0.010, and the Game View showing **the identical layout** to the runtime for seed 12345 — same lake, same blocks, same dots. Determinism across hosts, verified visually.

## Explain-it check

_(Superseded by the Nov 15 handover: Zac asked Dietrich to keep building the project solo. The three Lesson 11 questions stay on the record here — why raw RNG makes terrible terrain, why hash the lattice on demand instead of storing a grid, what each fBm octave adds — and the answers live in the Concept section above, in the form the lesson taught them.)_

## Next step

Lesson 12 — the editor console: a panel the engine can speak into (log lines from generation, the build, and the game), finishing Arc 2's foundations.
