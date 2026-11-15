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

_(recorded by Dietrich from the session, if anything does)_

## Verification

_(recorded by Dietrich from the session)_

## Explain-it check

_(Zac answers in chat; recorded here by Dietrich)_

## Next step

Lesson 12 — the editor console: a panel the engine can speak into (log lines from generation, the build, and the game), finishing Arc 2's foundations.
