# Devlog 034 — Gold Rush v0.1.0: The Package (Arc 7 Closes)

## Goal

Close Arc 7 by shipping the game: a script that turns the Runtime into a versioned, self-contained release zip anyone can unzip and play.

## Concept

- **A release is a *subset*, chosen deliberately.** The package holds the Runtime binary (renamed `GoldRush` — the game gets its own name on disk), only the assets the game itself loads, a default `engine.ini`, a player README, and a VERSION file. The editor, the editor-only click sound, the source, and the build tree stay home. Deciding what *doesn't* ship is most of packaging.
- **Static linking is a packaging feature.** This project builds SFML as static libraries, so the binary carries its engine with it — no bundled .so files, no launcher scripts, no "works on my machine". The remaining dependencies are the OS's own (X11/OpenGL on Linux), which is the honest floor for a native binary.
- **The script is the release process.** `tools/package_game.sh` configures, builds, stages, and zips in one command, with the version in exactly one place. A release you can rebuild with one command is a release you can patch; a folder you zipped by hand once is archaeology. `dist/` is gitignored — artifacts are outputs, not source.
- **Clean-room or it didn't happen.** The zip was extracted into an empty directory with no repo around it and launched from there. Assets resolve relative to the working directory (Lesson 20's rule), so the package's contract is "run from your own folder" — the test proves the contract holds with nothing else on the machine but the zip's contents.

## Code built

- `tools/package_game.sh` (version 0.1.0; build dir parameterised so the dev tree's cached dependencies can be reused).
- `tools/GoldRush-README.txt` — the player-facing readme (goal, controls, the synthesized-audio note, font credit).
- `.gitignore`: `dist/`.

## What broke

Nothing in the package. The clean-room run's log shows ALSA/PulseAudio errors — that's this rig having no audio device at all (known since Lesson 30), not a package defect; the game runs silently here and will find a sound card on any real machine.

## Verification

The script produced **GoldRush-0.1.0-linux.zip (1.9 MB)**. Clean-room test: unzipped into an empty /tmp directory, launched `./GoldRush` from inside it — the title screen rendered identically to the in-repo run (same seed, same world), with the real crate textures and the bundled font, proving every asset resolved from the package alone. Scope, stated plainly: this is the **Linux** build from Dietrich's rig. A Windows zip needs the same steps run in Zac's Windows build environment (plus its runtime libraries) — that's an Arc 9 packaging task, and the script's layout is the template for it.

## Explain-it check

_(Post-handover format: why does the release exclude files the repo considers assets? Why is the version in the script rather than the game? Why is dist/ gitignored if the zip is the arc's payoff? — the answers live in the Concept section above.)_

## Next step

Arc 8 — the third dimension: raw OpenGL. A new render path grows beside the SFML one (the 2D game keeps working untouched): shader pipeline, a spinning cube, then a camera flying over the tile world extruded into height. Zac's parallel LearnOpenGL follow-along meets the engine here.
