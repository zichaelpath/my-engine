# MyEngine

My own game engine, built from scratch in C++20, one lesson at a time — and documented as it goes. The end goal is to ship it as a portfolio piece: an engine library with an editor and a standalone runtime, the way Godot or Unity are shaped, at my own scale. Procedural generation is a foundation of the engine, not an add-on: the world is a deterministic function of a seed from the very first systems up.

**Stack:** C++20, CMake, SFML 3 (windowing/input for the 2D phase), Dear ImGui (docking build) + ImGui-SFML for the editor UI. Raw modern OpenGL arrives in Arc 8 for the 3D core.

**Status: Lesson 23 complete.** Arc 1 ✅ · Arc 2 ✅ · Arc 3 ✅ · Arc 4 ✅ · Arc 5 (the editor grows up) in progress — click-to-select is in, and a Hierarchy panel now lists every entity by name, driving the same selection. The Inspector (editable properties) is next. The full plan lives in [docs/ROADMAP.md](docs/ROADMAP.md).

![The editor, zoomed out over the 1600x1200 world](docs/shots/lesson-16-editor-zoom.png)

## What exists today

Three targets, built from one CMake project:

- **GameLib** — the engine/game library: the game world, deterministic procgen (master seed + named RNG streams), value noise with fBm, a logging system, config loading, a small math toolbox, data-driven entities with AABB collision, a camera (centre + visible height, follow with world-edge clamping), and a code-generated sprite sheet with frame animation (walk cycles, facing flips).
- **MyEngine** — the editor: dockable ImGui panels (Engine Debug, Procgen, Console, Game View), a resizable game view rendered to a texture, play/pause/step time controls, a live seed/noise control panel, a colour-coded console fed by the engine's log, and camera controls (follow toggle, zoom slider, mouse-wheel zoom, right-drag pan).
- **MyRuntime** — the standalone player: the same GameLib with no editor attached, its own window defaults, and a follow camera.

The world is currently a 1600×1200 procedurally generated continent (one batched vertex array, identical in editor and runtime for the same seed), populated by a player entity, crate entities, and scattered terrain blocks — all solid, all colliding.

## Build (Windows, VS Code)

```
cmake -B Build
cmake --build Build
```

Run the editor or the runtime **from the repo root** (asset paths like `assets/crate.tga` resolve relative to the working directory):

```
Build/Debug/MyEngine.exe
Build/Debug/MyRuntime.exe
```

## Controls

- **Move:** WASD or arrow keys (the player entity; diagonal movement is normalised, walls slide)
- **Quicksave / quickload:** F5 saves the scene to `scene.txt`, F9 loads it back (both hosts; the editor also has Scene buttons)
- **Runtime:** Esc closes
- **Editor:** Play/Pause/Step buttons drive game time; the Procgen panel applies seeds and reshuffles terrain/decor streams independently; the Camera follow checkbox, Zoom slider, mouse wheel over the Game View, and right-drag panning control the editor camera

## Configuration

Both hosts read an `engine.ini` next to the executable on startup (created with defaults if missing). Keys: `windowWidth`, `windowHeight`, `seed`, `noiseScale`, `playerSpeed`. Malformed lines are logged as warnings and skipped, never fatal.

## Documentation

- [docs/ROADMAP.md](docs/ROADMAP.md) — the ~80-lesson, nine-arc plan
- [docs/LESSON_BLOG.md](docs/LESSON_BLOG.md) — the narrative blog of every lesson
- [docs/devlog/](docs/devlog/) — one devlog per lesson: goal, concept, code built, what broke, verification
- [docs/shots/](docs/shots/) — the screenshot record, one or more per lesson

_Upkeep rule: every lesson updates the devlog, the lesson blog, and this README in the same commit as the code._
