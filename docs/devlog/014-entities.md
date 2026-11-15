# Devlog 014 — Entities (The Player Becomes a Citizen)

## Goal

End the player's special status. Introduce `Entity` — a plain data struct — have `Game` own a list of them, spawn the player and two crates through the same door, and move/draw everything by iterating data. Arc 3 opens.

## Concept

Until now the player was a *member variable with privileges*: its own shape, its own movement code, its own draw call. Every future feature (a second character, collision, an inspector) would have had to special-case it. An entity system inverts that:

- **An entity is data, not an object with behaviour.** `Entity` is a struct: name, position, size, colour, and a `playerControlled` flag. It has no methods and no SFML shape inside it. The rectangle you see is *derived* from the data at draw time.
- **Systems operate on entities from outside.** Movement is now a loop-shaped idea: read input, apply it to whichever entity is player-controlled. The input code no longer knows a special shape exists — swap the flag to a crate and the crate drives. Rendering is a system too: iterate the list, derive a rectangle per entity, draw (two passes, so the player stays on top).
- **Identity is an index.** `spawnEntity` returns the new entity's index in the vector, and `Game` remembers the player's index. Indices are stable because entities are never removed yet — a promise the code can keep today, and one Lesson-future (despawning) will have to renegotiate honestly (generational handles are the usual answer; that bridge comes when something actually despawns).

Why a struct and not full ECS? Because the honest comparison matters: a struct-of-fields per entity is simple, debuggable, and entirely sufficient until entities need *different subsets* of data (a crate has no input; a ghost has no collision). When that pressure arrives, the fields split into components. Designing for a problem you don't have yet is how engines die young; the roadmap grows this design under real pressure instead.

The crates, honestly noted: they are citizens but not yet *solid*. The player walks straight through them — collision is Lesson 15, and the walk-through is the proof that entities are currently pure data with rendering, no physics. One system at a time.

## Code built

- New `src/Entity.hpp`: the `Entity` struct (name, position, size, colour, playerControlled). Header-only data — no `.cpp`, so no CMake change (the Lesson 8 rule, applied).
- `Game`: `m_player` (a shape) is gone; replaced by `std::vector<Entity> m_entities` + the player's index. `spawnEntity(name, position, size, colour, playerControlled)` logs each spawn. The constructor spawns Player (375, 275), and two orange Crates at (200, 150) and (560, 380).
- `Game::update`: movement applies to the player-controlled entity, clamped by *its* size. `Game::render`: entities drawn from data, player pass last. `playerPosition()` reads the player entity — the editor panel never noticed the revolution.
- Editor: Engine Debug panel gains an "Entities: N" line.

## What broke

Nothing — the refactor compiled first try and both hosts ran. The one design point that needed care in code review: `spawnEntity` returns an index, not a reference, because a `std::vector` can reallocate on the next spawn and invalidate references. The index is the identity; the vector is the storage. (Worth recording precisely because it is invisible when it works.)

## Verification

Built clean on the Linux rig. Runtime screenshot (`docs/shots/lesson-14-runtime.png`): the seed-12345 continent with the bright green player at centre and both orange crates at their spawn points — (200, 150) upper-left, (560, 380) right-centre — all three rendered by the same data-driven loop. The runtime's stdout carries the three spawn lines ("Entity spawned: Player at (375, 275)", both crates), and the editor's Engine Debug panel reports "Entities: 3" while its player readout works unchanged through `playerPosition()` — the panel genuinely did not notice the rebuild underneath it.

## Explain-it check

_(Post-handover format: why is the entity a data struct instead of a class with methods? What breaks about index-as-identity the day something despawns? Why does the editor panel not care that the player was rebuilt underneath it? — the answers live in the Concept section above.)_

## Next step

Lesson 15 — AABB collision: entities (and the terrain blocks) become solid; the player stops at crate edges instead of ghosting through, resolved axis-by-axis so walls slide instead of stick.
