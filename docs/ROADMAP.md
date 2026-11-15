# MyEngine — Roadmap

_The living plan. Lesson counts are honest estimates, not promises; dogfooding (building a real game on the engine in Arc 7) is expected to reorder later arcs. Updated as arcs complete._

**Where we are:** Lessons 1–41 complete. Arc 1 ✅, Arc 2 ✅, Arc 3 ✅, Arc 4 ✅, Arc 5 ✅, Arc 6 ✅, Arc 7 ✅, Arc 8 ✅, Arc 9 in progress — the project file format (project.myengine) shipped; release packaging and the documentation pass remain. The engine today: `GameLib` + editor + runtime, a deterministic 1600×1200 procgen world stored as tile data, animated sprite entities that collide by hitbox, terrain-aware movement, file-backed textures and sounds with placeholder fallbacks, a full synthesized soundscape and streamed music, game rules (collection, win, run timer) in `Game`, a follow/zoom camera in both hosts, F5/F9 scene quicksave/quickload, and a full editor: selection, hierarchy, inspector, drag-move, spawn/delete, undo/redo, and play-test with restore. The engine today: `GameLib` + editor + runtime, a deterministic 1600×1200 procgen world stored as tile data, animated sprite entities that collide by hitbox, terrain-aware movement, file-backed textures and sounds with placeholder fallbacks, a full synthesized soundscape and streamed music, game rules (collection, win, run timer) in `Game`, a follow/zoom camera in both hosts, F5/F9 scene quicksave/quickload, and a full editor: selection, hierarchy, inspector, drag-move, spawn/delete, undo/redo, and play-test with restore.

**The honest scale:** Godot is ~25 years and hundreds of contributors. This roadmap does not chase feature-for-feature parity. The target is the depth a solo developer actually uses to ship real 2D games — rendering, entities, scenes, collision, audio, procgen, a real editor — every piece understood from the inside, plus the first 3D steps. That is roughly **80 lessons** to the end of Arc 9.

---

## Arc 1 — Editor shell & the split (Lessons 1–9) ✅ DONE

The loop, the clock, input → ImGui debug panel → game view in a RenderTexture → play/pause/step → docking → resizable view → game split out of main.cpp → game as a library, Editor + Runtime executables.

## Arc 2 — Core foundations (Lessons ~10–14)

The engine's bedrock, laid before the world grows:

- **Procgen foundation**: master seed + named RNG streams (deterministic forever — same seed, same world, in editor, runtime, and tests)
- **Noise**: value/Perlin noise built on the streams; a seed-regenerate panel in the editor
- **Console/logging**: an editor console panel the engine can speak into
- **Config & math utilities**: the small shared toolbox everything later will use

_Why procgen this early: it was a founding requirement of this engine, and determinism is cheap to have from the start and brutal to retrofit once saves, scenes, and gameplay depend on random calls scattered through the code._

## Arc 3 — The world gets things in it (Lessons ~15–22)

- Entities & components (grow from structs toward an ECS-lite design, honestly compared)
- A second entity; AABB collision
- A real camera: pan, zoom, follow — beyond "fit the view to the canvas"
- Sprites & textures; sprite-sheet animation

## Arc 4 — Scenes & data (Lessons ~23–28)

- Scene save/load (serialization to a file format of our own)
- Resource management: loading, caching, handles
- Tilemaps

## Arc 5 — The editor grows up (Lessons ~29–38)

- Hierarchy panel; selection
- Inspector: edit a selected entity's properties live
- Move tool / gizmos
- Undo/redo (command pattern)
- Play-in-editor with state restore (snapshot before Play, restore on Stop — the classic hard one)
- Asset browser; the Arc 2 console graduates here

## Arc 6 — Audio (Lessons ~39–42)

- SFML audio: sounds, music, a sound manager in the library
- Editor audio controls

## Arc 7 — A real game on top (Lessons ~43–52) — the dogfood milestone

- A small procgen game built *with* the engine, using Arc 2's seed core for level generation
- Game states & menus; input actions (rebindable, named — replacing raw key checks)
- Save files (which lean on Arc 2 determinism + Arc 4 serialization)
- Shipped through the Runtime

## Arc 8 — 3D / OpenGL (Lessons ~53–70)

- Raw modern OpenGL renderer alongside the SFML 2D path
- Shaders, meshes, a 3D camera
- First 3D scene running in both hosts

## Arc 9 — Ship the engine (Lessons ~71–80)

- A project file format; release packaging
- Documentation pass; public release as a portfolio piece
