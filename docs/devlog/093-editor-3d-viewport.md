# Devlog 093 — The Editor Grows a 3D Eye

## Goal

Arc 9's stretch: the editor and the GL host have been two programs that never share a screen. Put the 3D heightfield *inside* the editor — a "3D View" panel rendering the current scene in raw OpenGL, next to the 2D views it is painted in.

## Concept

- **Two extractions had to happen first, and both hosts are better for them.** The heightfield builder (`buildTerrain`, `tileHeight`, `tileSpec`) lived inside `gl_main.cpp`'s anonymous namespace; it is now `src/TerrainMesh.*` in the engine library, harness-pinned to reproduce the host's 18,604-triangle world exactly, and the standalone host renders pixel-identical frames through it (0 of 120,000 pixels differ). The shader compile/link helpers made the same trip into `src/GLProgram.*`. The rule as always: shared truth moves into the library; hosts keep only what is genuinely theirs.
- **The editor's window was already a GL context** — SFML made it one; ImGui-SFML draws with GL too. So `Viewport3D` loads the engine's own GL entry points (the same loader, a second consumer), renders the terrain into its own framebuffer with a depth renderbuffer attached, and the panel shows that framebuffer's colour texture through `ImGui::Image` (rows flipped — GL counts from the bottom, ImGui from the top). The render happens in the frame's pre-ImGui section, and the class restores what it touches (framebuffer, viewport, depth test, program, VAO, texture) because SFML's 2D drawing shares the context and assumes a tidy room.
- **Freshness without hooks.** The panel must rebuild its mesh when tiles change — but painting, scene loads, procgen and undo all change tiles through different code paths. Rather than a hook in each, the editor hashes the tile grid every frame (an FNV pass over 4,800 tiles, lost in the noise of a frame) and marks the viewport dirty when the hash moves. One mechanism, every path covered, including the ones nobody has written yet.
- **The camera is a slow orbit on purpose.** No input in this lesson: the panel is a living preview while both hands are busy in the 2D views. Its program is the terrain's Lambert + Blinn-Phong minus shadows and fog — a workbench light, honestly labelled in the source as a copy that a future shader library should absorb.
- **Verification without a mouse.** This rig has no GUI driver, so the panel's checkbox cannot be clicked headlessly. The viewport *class* was therefore verified end-to-end by a probe: a hidden SFML window (the same context recipe as the editor), a real `Viewport3D`, the default world — read back and counted: 76,835 horizon pixels, 37,840 green, 21,187 water-blue, and the saved frame (below) shows the lit heightfield from the orbit camera. The editor binary itself smoke-runs clean. What remains genuinely unverified is only the ImGui checkbox click path — Zac's first toggle will confirm it, and the panel degrades to a text note if GL ever fails to initialise.

## Code built

- `src/TerrainMesh.hpp/.cpp` — the heightfield rules, extracted to the engine library; `src/GLProgram.hpp/.cpp` — shader compile/link, extracted to the GL library.
- `src/Viewport3D.hpp/.cpp` — the FBO viewport: lazy GL init, resizable colour+depth target, dirty-flag rebuilds through the shared builder, the orbit camera, state hygiene around SFML.
- `src/GLLoader.hpp/.cpp` — renderbuffer entry points, `glDeleteFramebuffers`/`glDeleteRenderbuffers`, and `glDeleteProgram` (the one delete the loader never had; its shader sibling was there all along).
- `src/main.cpp` — the "3D View (heightfield)" checkbox, the per-frame tile hash, the render call, and the panel.
- `CMakeLists.txt` — TerrainMesh joins GameLib; GLProgram joins the GL library; the editor links the GL library and compiles Viewport3D.
- `tests/tests_main.cpp` — the TerrainMesh pin (suite now 68).

## Verification

- Harness: **68 passed, 0 failed.** Standalone host: pixel-identical frame after both extractions.
- The probe frame (`docs/shots/lesson-93-viewport-probe.png`): the whole Gold Rush world from the viewport's orbit camera — specular water, lit plateaus, horizon clear — rendered by the class the editor panel displays.
- The editor builds and runs; the 3D View checkbox sits next to Game View 2's in Engine Debug.

## Explain-it check

_(Post-handover format: why does the viewport render into a framebuffer instead of drawing directly in the panel's screen region? Why hash the tiles instead of hooking every edit path? Why must the viewport restore GL state it borrowed? — the answers live in the Concept section above.)_

## Next step

Lesson 94 closes Arc 9: the full-suite pass, the arc test, engine **1.8.0**, and the roadmap brought up to date — then Arc 10, the ship.
