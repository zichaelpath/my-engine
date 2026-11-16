# Changelog

## 1.6.0 — 2026-11-16

The engine's games grow from places into journeys. Where 1.5.0 shipped two standalone worlds, 1.6.0 gives a project many scenes, a campaign across them, screens built from shared parts, and a player whose progress survives the night.

**Multi-scene projects.** A project card's `scene=` line is repeatable: an ordered level list, the first its start. Travel is a naming convention, not a format change — an entity named `Exit:<scene>` is a door, `Spawn:<scene-you-came-from>` (or plain `Spawn`) is the landing, and spawn markers show for authors in the editor while staying invisible in shipped games.

**Campaigns.** A `Campaign` model tracks an ordered level sequence: levels unlock in order, completion advances to the next unfinished level, and replaying an old level never rewinds progress. A level's finish line is an entity named `Goal`, touched like a door. The Runtime plays campaigns end to end — title-screen level select with locked and done states, a "Level N of M" HUD, and a Campaign Complete screen. A complete two-level demo campaign ships in `games/two-rooms/`.

**Shared UI widgets.** Every Runtime screen is rebuilt from `ui::Label`, `ui::Button` and `ui::Panel`: state-drawn buttons (resting, hovered, gold-ringed selected, dimmed disabled), screen actions shared between the keyboard and new mouse routing, so Enter and clicking Play are one code path.

**Save files.** A versioned `save.dat` in the project folder holds the player's state: campaign progress (restored only when the saved level list still matches the project), Gold Rush best times per seed, Temple Run's best height, and per-player settings — the first being camera shake, toggled with K on the title screen. Progress is written at the moment it happens, and bests only move one way.

The test harness grows to 60 checks, including an end-to-end campaign played across a simulated restart. Both games repackage at 1.6.0 and pass the clean-room boot.

Every step is documented in `docs/devlog/` (82 lessons) and retold in `docs/LESSON_BLOG.md`.

## 1.5.0 — 2026-11-15

The engine ships a second game. Where 1.0.0 proved one game could be built on MyEngine, 1.5.0 proves the engine is the reusable part: a structurally different game — a vertical endless climber instead of a top-down collector — built from a project folder, data files, and host wiring, with no changes to the game's core systems to make it fit.

**The new game — Temple Run.** An endless vertical climb in `games/temple-run/`, packaged standalone (`dist/TempleRun-1.5.0-linux.zip`, clean-room verified). Authored chunks are dealt like cards from the project's seed — every chunk enters and exits on the same landing, so any deal is climbable — and stream endlessly through a fixed window as a pressure line rises behind the player. Height is the score; the line is the clock. Title, climb, caught and restart screens run in the same Runtime that ships Gold Rush (which is also repackaged at 1.5.0).

**The project format grows up.** A project is a folder; its card now declares what kind of game it is (`mode=scene|ascent`, `chunks=`) with additive format-1 keys — old cards load unchanged. The Runtime takes a project folder as an argument and boots whatever game lives there. Projects have a FileSystem panel and file browser in the editor, collision-safe asset import, and scenes stored as files inside the project, with resources resolved project-relatively.

**Scripting: Lua, per game.** Lua 5.4.7 is embedded and hand-bound: games carry their own scripts (entity API v1, per-entity state, hot reload while the game runs), entity types are data, and scenes serialise to format v2. A complete NPC patrol ships as pure Lua in the patrol demo — zero engine code.

**Feel and physics.** An easing/tween and particle layer, trauma-based camera shake with hit-stop, and a Feel panel that tunes the game's personality as data. Platformer physics arrived alongside: gravity, coyote time, jump buffering and cutting, tile collision, one-way and moving platforms — the toolkit Temple Run is built from.

**Fixes folded in.** The left-facing sprite offset (collision was never wrong; the picture was), the vendored KHR header that un-breaks the Windows build, a 55-check test harness (`MyTests`) that runs headlessly, and CI on Linux and Windows. (These were the planned 1.0.1 patch; they ship inside 1.5.0 instead.)

Every step is documented in `docs/devlog/` (77 lessons) and retold in `docs/LESSON_BLOG.md`.

## 1.0.0 — 2026-11-15

The first release: a small but complete game engine, built lesson by lesson from an empty `main()`, with a real game shipped on it.

**The game — Gold Rush.** A complete 2D collection game (title, pause, win screens; run timer; sound and music) built in the engine's Runtime, packaged as a standalone zip. Its world is procedural: one seed, one deterministic world — terrain, water that slows you, 40 gold dots, every time.

**The editor.** A dockable ImGui editor over the same simulation: live game view, hierarchy, inspector, console, entity picking/dragging/spawning/deleting, undo/redo, scene save/load, and play-in-editor with exact state restore.

**The engine (GameLib).** Seeded procedural generation (named random streams, value noise + fBm), entities with sprite animation and AABB collision, a camera, tile terrain with per-type movement, scene serialization, resource caches, synthesized audio (sounds and a music loop generated in code — no audio files were recorded or licensed), a project file format (`project.myengine`), and per-machine config.

**The 3D host (MyGL).** Raw OpenGL 3.3 with a hand-written function loader (no GLAD): shaders, MVP matrices, a fly camera, the 2D tile world extruded into a sun-lit heightfield, and textured meshes — the game's crates stand in the 3D world at their 2D positions.

**Platforms.** Builds on Linux and Windows from one CMake file; dependencies (SFML 3.0.2, ImGui, ImGui-SFML) are fetched and built by CMake itself. See `docs/WINDOWS_BUILD.md` for the Windows walkthrough.

Every step of the build is documented in `docs/devlog/` (41 lessons) and retold in `docs/LESSON_BLOG.md`.
