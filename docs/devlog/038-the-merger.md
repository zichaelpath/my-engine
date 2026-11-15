# Devlog 038 — The Merger: The Tile World, Extruded

## Goal

Bring the two halves of the engine together: MyGL builds a 3D heightfield from the *same* `Game` world Gold Rush plays — same seed, same tile data, same palette — and the fly camera goes sightseeing over it.

## Concept

- **One world, two renderers.** MyGL constructs a plain `Game` (default seed 12345) and reads its tile grid through two small additions: `tileAt(tx, ty)` / `tilesX/Y` (grid-space queries) and `Game::tileColor`, hoisted out of the 2D field builder where it used to be a local lambda. The palette is now a single table both renderers read — the 3D view cannot drift from the 2D map because there is nothing to drift *from*. `tileAtWorld` delegates to `tileAt` now; one bounds check, two doors.
- **Height is a reading of type.** DeepWater −0.55, Water −0.25, Sand 0.05, Grass 0.35, Forest 0.65, Rock 1.15 — the classification the movement code already uses becomes elevation. Water sits in its basin, shores step up through sand, forest stands above grass, rock caps the world. The mesh is one flat top per tile (80×60 of them) plus a **skirt** wherever a tile stands above a neighbour — the higher tile owns the wall, so every step gets exactly one — drawn in the top's colour darkened to 0.72, a cheap depth cue until real lighting arrives. 18,604 triangles, one draw call, identity model matrix: the mesh is built in world coordinates.
- **A determinism scare, investigated properly.** A pixel comparison against the Lesson 32 Runtime screenshot showed most sampled tiles "different", which would have meant generation changed. It hadn't: a probe program printed a checksum of the whole grid from today's code and from Lesson 32's code compiled out of a git worktree — **identical (246701663), cell for cell**. The screenshot was the liar: it had been captured through a leftover editor `engine.ini` with a different window size, so its camera framed a different slice of the world than the comparison assumed. The probe stays the receipt: same seed, same world, provably.

## Code built

- `Game` — `tileAt`, `tilesX/tilesY`, static `tileColor` (the 2D render calls it unchanged; its output is byte-identical).
- `src/gl_main.cpp` — `tileHeight` + `buildTerrain` (tops + skirts, one triangle list), terrain VAO, one draw; camera starts high over the world; the cube field retires (its lessons are in the git history and the loader/Mat4 it built).
- Rig note: the build machine needed its OpenGL dev packages completed (`libxfixes-dev`, `libgl1-mesa-dev`) after a header cascade exposed that GLLoader had been finding `<GL/gl.h>` through a side path; CMake now uses `find_package(OpenGL)` and links `OpenGL::GL`, which carries the include path properly on any machine.

## What broke

The build, twice, both times in the environment rather than the code: SFML's X11 backend needed `Xfixes.h`, and FindOpenGL needed a real `libGLX.so` — fixed by installing the two dev packages above, and by the `find_package(OpenGL)` change, which is the durable fix. Xvfb had also died between sessions and was restarted.

## Verification

The screenshots show the world Zac knows from above — the big central lake, its sand shoreline, forest blocks, grey rock outcrops standing highest — from a high three-quarter view and from a low pass where the terraces and skirts read as solid steps with no see-through gaps. The build log reports "Terrain built: 18604 triangles from a 80x60 tile grid", the camera's flight log matches the low shot's viewpoint, and the grid checksum (above) proves the 2D game underneath is untouched.

## Explain-it check

_(Post-handover format: why hoist the palette instead of copying the numbers into the terrain builder? Why does the higher tile own the skirt? Why did the checksum probe settle the screenshot argument when eyeballing couldn't? — the answers live in the Concept section above.)_

## Next step

Arc 8 continues: the heightfield is flat-shaded by palette alone — next it gets real light (normals + a directional sun in the shader), which is what turns steps into terrain. Arc 9 after that: the project format and the Windows package.
