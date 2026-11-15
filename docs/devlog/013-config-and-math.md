# Devlog 013 — Config & Math Utilities (The Toolbox, and Arc 2 Closes)

## Goal

Two pieces of shared infrastructure every later system will stand on: a `Config` the hosts load (engine.ini), and a `Math.hpp` toolbox — proven immediately by refactoring Noise and movement onto it, and by having both hosts take their seed, noise scale, player speed, and window size from the same file.

## Concept

**Config is a host concern with engine manners.** The file is plain `key = value` text with `#` comments. Hosts *register defaults* first, then load the file over them, then read typed values (`getInt` / `getFloat` / `getBool` / `getString`) with fallbacks. Three failure policies, all deliberate:

- **Missing file is not an error.** The merged defaults are written out to `engine.ini`, so a first run creates the very file the user can then edit. The console says so.
- **A malformed line is a warning, not a crash.** It is logged with its line number and skipped; the rest of the file still applies. A typo in a text file must never take the engine down — and Lesson 12's console is what makes this policy visible instead of silent.
- **A garbage value falls back.** `getInt("seed")` on `seed = banana` returns the fallback, via exception-safe parsing. The config layer absorbs the mistake; callers never see it.

**A toolbox earns its place by adoption, not by existing.** `Math.hpp` is header-only (one-liners the compiler inlines) and deliberately small: `lerp`, `remap`, `smoothstep01`, `normalize`. Two of them were adopted the same lesson they were written: Noise's `sample` now calls `math::lerp` and `math::smoothstep01` (its private copies are deleted), and movement calls `math::normalize`.

**Why the engine normalises its own vectors:** SFML 3 has `Vector2::normalized()` — and checking its source (the standing rule) shows it *asserts on a zero vector*, which in a release build becomes a divide-by-zero. "No input held" is the most common direction a game ever computes, so the toolbox's `normalize` guards it: zero in, zero out. Reading the library before adopting it is the whole point.

One config, two hosts: the editor and the runtime register the same keys with different window-size defaults (1280×720 vs 800×600). Run either from a folder containing an `engine.ini` and both generate the same world from it.

## Code built

- New `src/Math.hpp` (header-only, namespace `math`): `lerp`, `remap`, `smoothstep01`, `normalize` (zero-guarded).
- New `src/Config.hpp` / `src/Config.cpp` (in GameLib): defaults registration, `load` / `save`, typed getters; load/save/malformed events go through `Log`.
- Refactors: `Noise::sample` onto `math::lerp` / `math::smoothstep01`; `Game::update` onto `math::normalize`.
- Hosts: both `main.cpp` and `runtime_main.cpp` register defaults (`windowWidth`, `windowHeight`, `seed`, `noiseScale`, `playerSpeed`), load `engine.ini` (writing it if missing), size their windows from it, and apply seed / noise scale / speed to the game. Editor panel statics (seed input, noise slider) now initialise from the game's actual values, so config and panel can never disagree at startup.
- `Game`: new `noiseFrequency()` getter. `CMakeLists.txt`: `src/Config.cpp` added. `.gitignore`: `engine.ini` (it is a per-machine file the engine writes).

## What broke

Nothing in the build — but the pre-flight check caught a trap before it shipped: SFML 3's `Vector2::normalized()` looks like the obvious choice for movement, and its source (SFML 3.0.2, `Vector2.cpp`) asserts on a zero vector, which release builds turn into a divide-by-zero. The engine's zero-guarded `math::normalize` exists because the library was read before it was adopted — the standing rule paying for itself again.

## Verification

Built clean on the Linux rig. Three runs:

- **Editor with a hand-written `engine.ini`** (`seed = 777`, `noiseScale = 0.016`, `playerSpeed = 260`, plus one deliberately broken line) — screenshot `docs/shots/lesson-13-editor.png`: the Procgen panel opens on seed 777 with the noise slider at 0.016 and the speed slider at 260 (panels initialised from the game, game from the config), the console shows "Seed set to 777" and the field regenerating at noise scale 0.016, and the continent is seed 777's — visibly different from, and busier than, seed 12345's.
- **Runtime, same folder, stdout captured**: `[warning] Config: ignoring malformed line 5: this line is broken`, then `Config: loaded 3 values`, then the identical application sequence (seed 777, field at 0.016). The warning path and the two-hosts-one-config story both proven in text.
- **Runtime in an empty folder**: `Config: engine.ini not found, using defaults` + `Config: wrote defaults to engine.ini`, and the written file contains exactly the runtime's registered defaults (800×600 window among them).

## Explain-it check

_(Post-handover format: why do the hosts register defaults instead of the Config class hardcoding them? Why is a malformed config line a warning rather than an error? Why does the engine keep its own `normalize` when SFML ships one? — the answers live in the Concept section above.)_

## Next step

Arc 3 begins — Lesson 14: entities. The player stops being a special shape inside `Game` and becomes the first citizen of a small entity system, with a second entity to prove it is a system.
