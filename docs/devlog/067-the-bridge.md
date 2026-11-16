# Devlog 067 — The Bridge: An API a Script Can Hold

## Goal

Give scripts hands. The host learns the **engine API v1** — a small `entity` table of hand-bound C functions plus global `log()` and `time()` — and the module/instance model that everything else in Arc 5 builds on: a script file returns a table of functions (the Lua module idiom); an *instance* binds that module to one entity and owns a per-entity **state table** (`self`) where the script keeps its memory.

## Concept

- **State lives in Lua, identity in a name.** The state table carries the entity's name in `__name`; bindings resolve it against the game's entity list at call time. Scripts write `self.speed`, `self.t` — per-entity memory with no C++ schema, no serialization format, no registration. The known simplification, stated plainly: addressing is by name (first match), so renaming an entity in the editor will orphan its script's view of it; instances are (re)bound when a scene loads, which is also when names settle. Stable entity IDs are the grown-up answer and a deliberate non-goal for v1.
- **The Game travels as an upvalue.** Each binding is a plain C function closure over a lightuserdata `Game*` — one binding set per install, no C++ globals, no singleton. `installEntityApi(game)` is a sentence you can point at: this host now speaks for this game.
- **The API is deliberately tiny.** `getPosition`/`setPosition`/`move`, `getVelocity`/`setVelocity`, `isGrounded`, `log`, `time`. That is enough to patrol, chase, hover, and time behaviour — and small enough that a rookie can hold the whole surface in their head. It grows by need (later arcs), never by speculation.
- **A missing `onUpdate` is not an error.** `callUpdate` on a module without one returns true quietly — scripts may exist for other callbacks later. An `onUpdate` that *errors* returns false with the message logged, and the host — proven in the harness — answers the next call.

## Code built

- `src/ScriptHost.hpp/.cpp` — `installEntityApi`, `loadModule` / `loadModuleFromString` (a module must return a table; anything else is a logged refusal), `createInstance`, `callUpdate`; the eight binding functions, resolved through the state table's `__name`.
- `tests/tests_main.cpp` — the bridge check (suite now 46).

## Verification

- Harness: **46 passed, 0 failed** — a string-loaded module's `onUpdate` moves the target crate exactly +30 x over 60 calls at dt=1/60; the instance's `self.t` accumulates to exactly 1.0 (read back through the state the script kept); an erroring module returns false and the host evaluates afterwards.
- Probe: the same mover against the real world — crate x 200.0 → 230.0, rendered before/after.
- Screenshot: `docs/shots/lesson-67-bridge.png` — the two frames, Lua's work visible.
- Rig note: the probe link line now needs `liblua.a` (the hand-rolled probe recipes predate Lua) — the first probe binary was a truncated artifact of the failed link, diagnosed by its undefined references rather than trusted.

## Explain-it check

_(Post-handover format: why does the state table carry the entity's name instead of a pointer or an index? Why is the API surface kept this small for v1? Why is a missing `onUpdate` a quiet success while an erroring one is a logged failure? — the answers live in the Concept section above.)_

## Next step

Lesson 68: entity types — the data half of the model. Entities gain a `type` name and a `script` path; a type registry gives a new entity its defaults (size, colour, solidity, movement model, script) from one definition.
