# Devlog 041 — The Project File: An Identity Card for a Game

## Goal

Open Arc 9 (ship the engine) with its foundation: a project file format. Until now the game's identity was scattered — a hardcoded title, a seed in a per-machine config, a scene filename baked into two hosts. One file now says what game this is.

## Concept

- **`project.myengine` is the game's identity card.** Four meaningful lines — `format=1`, `name=`, `seed=`, `scene=` — in the same `key=value` family as `engine.ini` and `scene.txt`, because an engine's formats should feel like one language. The split of duties is the design: the project file *travels with the game* and defines it (name, world seed, start scene); `engine.ini` stays on the player's machine and describes *their* setup (window size, volume, speed). When both speak, the project wins on the things it defines — a shipped game's world shouldn't change because a settings file says so.
- **The title screen is data now.** The Runtime takes its window title and its big title-screen name from the project (uppercased for the shout), its seed from the project, and loads the named start scene when one exists. A fresh checkout has no `scene.txt` yet — that's a normal state, checked for quietly, not an error. The Editor reads the same file for identity (its window title carries the game's name) but deliberately does *not* apply the seed: experimenting with worlds in the editor shouldn't require editing the shipped file.
- **Format versioning is a promise kept early.** The loader is all-or-nothing, like the scene loader: a missing `format=` line, or a version this engine doesn't read, fails the whole load loudly and leaves the output untouched. A project written by a future engine must never half-load in this one and produce a subtly wrong game.
- **Unknown keys warn and pass.** Forward compatibility in the other direction: a format-1 file with a key this build doesn't know still loads, with the key named in the log. Strict about versions, lenient about additions.

## Code built

- `src/Project.hpp/.cpp` (GameLib) — the `Project` struct and `loadProject`: trimmed `key=value` parsing, `#` comments, the version gate, all-or-nothing commit.
- `project.myengine` at the repo root — Gold Rush's card (seed 12345, scene scene.txt).
- `src/runtime_main.cpp` — project read first; window title, title-screen text, seed, and start scene all flow from it, with the no-project fallbacks unchanged.
- `src/main.cpp` — the Editor loads it for identity (window title), and logs it.
- `tools/package_game.sh` — the card ships in the Gold Rush package.

## What broke

Nothing in the build. One design wobble worth recording: my first instinct was to have the Editor apply the project's seed too, for symmetry — wrong, for the reason above. Symmetry is not a user; the two hosts want different things from the same file.

## Verification

Two runs, one screenshot each. From the repo root, the shipped card drives the familiar game — the log reads "Project loaded: Gold Rush (format 1, seed 12345, scene scene.txt)" and the title screen is unchanged, because the file describes the game that already existed. From a scratch folder, a four-line test card (`name=Star Salvage`, `seed=777`) produces a visibly different game: the screenshot shows STAR SALVAGE over a seed-777 world, and the window itself was findable by its project name. Same binary, different identity — that's the format working.

## Explain-it check

_(Post-handover format: why do the project file and engine.ini both exist — what belongs in each, and who wins a conflict? Why gate on an exact format version but tolerate unknown keys? Why doesn't the Editor apply the seed? — the answers live in the Concept section above.)_

## Next step

Arc 9 continues: the release pass — version numbers that mean something (the engine gets a version the hosts report), a Windows build-and-package walkthrough written for Zac's machine (the one piece of this arc only he can execute), and the documentation/release checklist that turns this repository into the portfolio piece it was always meant to be.
