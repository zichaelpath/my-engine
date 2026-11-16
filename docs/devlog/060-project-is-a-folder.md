# Devlog 060 — The Project Is a Place

## Goal

Arc 4 opens by changing what a project *is*. Until now `project.myengine` was a file the hosts found in whatever directory they happened to launch from. Now a project is **a folder**: the file lives inside it alongside the standard subfolders (`assets/`, `scenes/`, `scripts/`), the `Project` struct knows its root, and every path inside the project resolves against that root. This is the north star's first pillar, laid as plumbing before panels.

## Concept

- **A root makes paths honest.** Gold Rush's assets were found by launch-directory luck (the Lesson 44 magenta-crate incident was exactly this: launch from the wrong folder, lose your textures). With a root, "the scene this project starts from" means *a file inside the project*, wherever the editor was launched from — `projectPath` joins root + relative, and the runtime now resolves its start scene that way. An empty root preserves the old behaviour exactly, so nothing that worked breaks.
- **Creating refuses to clobber.** `createProjectFolder` makes the folder, the three subfolders, and a fresh identity card — and returns false if a project already lives there. Creating-over is a different verb from opening, and an engine that silently overwrites a project's identity card on a misclick is an engine that loses someone's game. The refusal is logged, loud, and tested.
- **Save is the missing half, now written.** `Project` had a loader since Lesson 41 and no saver — the file was effectively read-only, edited by hand. `saveProject` writes the same format 1 (no format change: the file's *meaning* grew a root, its *syntax* didn't), and the round-trip is pinned by the harness: create → read back → rename → save → read back again.
- **The repo becomes its own first citizen.** Both hosts now open the project through the folder API (root "."), and the repo gains its `scenes/` and `scripts/` folders beside the existing `assets/` — Gold Rush's project, laid out the way every future project will be.

## Code built

- `src/Project.hpp/.cpp` — `Project::root`; `loadProjectFolder`, `createProjectFolder`, `saveProject`, `projectPath`; one shared writer for the format-1 file.
- `src/main.cpp`, `src/runtime_main.cpp` — hosts open the folder API; the runtime resolves the start scene through the root.
- `scenes/.gitkeep`, `scripts/.gitkeep` — the standard layout, present in the repo.
- `tests/tests_main.cpp` — one new check, all in a temp directory (suite now 39).

## Verification

- Harness: **39 passed, 0 failed** — folder created with the full layout, read back with root and identity intact, second create refused, rename saved and re-read, `projectPath` joining correctly (and passing through untouched when there is no root).
- Probe: a real `DemoGame` place created on disk — card, three subfolders, and a scene path resolving inside it.
- Screenshot: `docs/shots/lesson-60-project-folder.png` — the created layout and the resolution rule.

## Explain-it check

_(Post-handover format: why does creating refuse when a project exists instead of just overwriting the card? Why does an empty root leave paths untouched instead of failing? Why did the file format stay at version 1? — the answers live in the Concept section above.)_

## Next step

Lesson 61: the FileSystem panel — the editor shows the project's tree, so the place Lesson 60 defined becomes a place you can *see*.
