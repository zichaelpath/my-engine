# Devlog 068 — Types: The Data Half

## Goal

Give entities a *kind*. An entity now carries a `type` name and a `script` path; a **type registry** in the Game holds definitions — size, colour, solidity, movement model, sprite, script — and `spawnTyped` / `applyType` stamp those defaults onto entities. Definitions live in code or in a `types` text file written in the engine's key=value family. This is the data half of the scripting model; Lesson 67 built the behaviour half, and Lesson 69 will persist both in scene format v2.

## Concept

- **A scene stores a name, not a copy.** An entity records only its type's *name*; the definition lives once, in the registry. Rebalance every Guard in a game — bigger, faster-modelled, new script — by editing one definition and reloading, instead of hunting through every scene that ever placed one. This is the same economy prefabs sell in big engines, at the scale this one can honestly support.
- **The types file speaks the engine's family language.** `type=` opens a block; `size=`, `color=`, `solid=`, `model=`, `sprite=`, `script=` fill it; unknown keys warn and are ignored, exactly like the project file and engine.ini. One text convention across the whole engine means a rookie learns it once. (Parsing note: `model=platformer` is the only stringly-typed field, and it maps to the same enum the Physics panel switches.)
- **Redefinition replaces, never duplicates.** `defineType` with an existing name overwrites the definition — live-rebalancing a type in the editor or a test must not accrete twins that `findType` then has to disambiguate. The harness pins this: redefine Guard at 48×48 and the registry's Guard *is* 48×48.
- **Failure stays a value.** Spawning or applying an unknown type logs and returns false; nothing half-stamps. An entity whose type is missing keeps its own fields — a scene from a game whose types file you don't have still loads and renders (Lesson 69 keeps this property in the format).

## Code built

- `src/Entity.hpp` — `Entity::type`, `Entity::script`; the `EntityType` definition struct.
- `src/Game.hpp/.cpp` — the registry: `defineType`, `findType`, `applyType` (stamps + re-derives the sprite hitbox), `spawnTyped`, `loadTypes`.
- `tests/tests_main.cpp` — the types check (suite now 47).

## Verification

- Harness: **47 passed, 0 failed** — a code-defined Guard stamps size/colour/model/script exactly; a two-type file loads with fields parsed (including `solid=0` and the platformer mapping); unknown types fail cleanly; redefinition replaces. (One aborted run on the way was the rig's Xvfb dying mid-suite — the display, not the code; the rerun on a fresh display is the green one recorded here.)
- Probe: two Guards spawned from one definition, rendered, and read back — both 44×44, both carrying `scripts/guard.lua`.
- Screenshot: `docs/shots/lesson-68-types.png` — the two stamped Guards beside the player.

## Explain-it check

_(Post-handover format: why does a scene store only the type's name instead of the whole definition per entity? Why does redefining a type replace rather than append? Why do unknown keys in a types file warn instead of failing the load? — the answers live in the Concept section above.)_

## Next step

Lesson 69: scene format v2 — scenes persist the type, the script, and the Arc 3 fields that have been waiting (movement model, platform patrols), while v1 files keep loading with defaults.
