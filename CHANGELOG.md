# Changelog

## 1.0.0 — 2026-11-15

The first release: a small but complete game engine, built lesson by lesson from an empty `main()`, with a real game shipped on it.

**The game — Gold Rush.** A complete 2D collection game (title, pause, win screens; run timer; sound and music) built in the engine's Runtime, packaged as a standalone zip. Its world is procedural: one seed, one deterministic world — terrain, water that slows you, 40 gold dots, every time.

**The editor.** A dockable ImGui editor over the same simulation: live game view, hierarchy, inspector, console, entity picking/dragging/spawning/deleting, undo/redo, scene save/load, and play-in-editor with exact state restore.

**The engine (GameLib).** Seeded procedural generation (named random streams, value noise + fBm), entities with sprite animation and AABB collision, a camera, tile terrain with per-type movement, scene serialization, resource caches, synthesized audio (sounds and a music loop generated in code — no audio files were recorded or licensed), a project file format (`project.myengine`), and per-machine config.

**The 3D host (MyGL).** Raw OpenGL 3.3 with a hand-written function loader (no GLAD): shaders, MVP matrices, a fly camera, the 2D tile world extruded into a sun-lit heightfield, and textured meshes — the game's crates stand in the 3D world at their 2D positions.

**Platforms.** Builds on Linux and Windows from one CMake file; dependencies (SFML 3.0.2, ImGui, ImGui-SFML) are fetched and built by CMake itself. See `docs/WINDOWS_BUILD.md` for the Windows walkthrough.

Every step of the build is documented in `docs/devlog/` (41 lessons) and retold in `docs/LESSON_BLOG.md`.
