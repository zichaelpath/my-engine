# Devlog 065 — Resources Have an Address: Arc 4 Closes

## Goal

Close Arc 4 by making the project's resources *addressable*: everything inside the project is named by its project-relative path, and consumers resolve that path against the root. The visible proof is the Project panel's new **image preview**: select an imported image and the panel shows it — loaded through `projectPath`, not through whatever directory the editor happened to launch from.

## Concept

- **One resolution rule, everywhere.** Scenes load through it (Lesson 60/64), imports report in it (63), the panel lists in it (61), and now resources load through it. The rule is three lines — root + relative, empty root passes through — and its boringness is the achievement: there is no second way to name a file inside a project, so there is no second way for a path bug to happen.
- **The preview is a cache of one, keyed by selection.** The texture reloads only when the selected file changes; a failed load shows "(preview unavailable)" instead of retrying every frame or, worse, showing the previous file's image under a new name. Small disciplines, but a panel that lies about which file it is showing is worse than no panel.
- **The proof runs from the wrong directory on purpose.** The harness's new check writes a real PNG, imports it into a temp project, and loads it as a texture purely by resolving the landed relative path — while the test process's working directory is somewhere else entirely. That is exactly the Lesson 44 magenta-crate failure, inverted into a guarantee: resources are found by *address*, not by launch luck.

## Arc 4, in retrospect

The north star's first pillar stands: a project is a folder with a standard layout (60), visible in the editor (61), openable through a portable browser (62), able to absorb assets by import without ever overwriting (63), with scenes as first-class files the editor tracks, saves, and reloads (64), and resources addressed project-relatively end to end (65). A rookie can now create a place, put things in it, and find them again — the "standard engine feel" the north star asks for, in its first concrete form.

## Code built

- `src/main.cpp` — the Project panel's image preview (png/jpg/jpeg/bmp/tga), loaded via `projectPath`, dimensions readout, fit-to-panel scaling.
- `tests/tests_main.cpp` — the resolution proof (suite now 44).

## Verification

- Harness: **44 passed, 0 failed** — the imported 6×4 image loads through its project-relative path from an unrelated working directory, size exact.
- Probe: a 64×48 gradient imported into a real project and loaded back through the root (screenshot).
- The editor runs with the preview in its panel code.
- Screenshot: `docs/shots/lesson-65-preview.png` — the imported gradient, loaded by address.

## Explain-it check

_(Post-handover format: why is one shared resolution rule worth more than each call site joining paths itself? Why does the preview reload only on selection change, and show an explicit "unavailable" instead of the last good image? — the answers live in the Concept section above.)_

## Next step

**Arc 5 — Scripting**, the north star's second pillar: Lua, embedded from source and hand-bound through its plain C API (Zac's decision, Nov 15), entity *types* as the data half, scene format v2, and a hot-reloading script host — proven by an NPC whose behaviour lives entirely in a `.lua` file in the project's `scripts/` folder. The folder Lesson 60 created for exactly this is waiting.
