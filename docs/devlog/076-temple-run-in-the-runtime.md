# Devlog 076 — Temple Run in the Runtime

## Goal

Make the second game a game: the runtime boots Temple Run from its own project folder and plays it end to end — title, climb, caught, score, climb again — while Gold Rush plays exactly as it always has, from the same binary.

## Concept

- **The project card learns to say what kind of game it is.** Format 1 gains two additive keys — `mode` (`scene` by default, `ascent` for the climb) and `chunks` (the chunk file's project-relative path). Additive means old cards are untouched by the change: a card without the keys is a scene game, exactly as before, and `saveProject` writes the new keys only when they carry information, so a scene game's saved card stays byte-for-byte the card it has always been. Temple Run's card now declares `mode=ascent` and `chunks=chunks.txt`.
- **One binary, any project.** The runtime takes an optional project-folder argument — `MyRuntime games/temple-run` — because Lesson 60 made a project a place, and a place can be named. No argument still means the working directory's project: Gold Rush's boot is unchanged, keystroke for keystroke.
- **The climb's frame mirrors the win's.** The runtime's screen machine gains one state, `Caught`, entered from Playing the frame `Game::caught()` turns true, with the world staying alive behind it exactly as it does behind the win. Restart is R, and it re-deals *the same temple*: `buildAscent(project.seed, 5)` again. That is a deliberate design stance — the project seed is this copy of the game's mountain, and mastery means learning *this* climb. (Gold Rush's R deals a fresh random world; the two games disagree about what a restart means, and both are right.)
- **The HUD tells the climb's truth.** In ascent mode the gold-and-time line becomes a single number — `Height: N` — updated live, and the caught screen reports the final height the same way. The title subtitle names the game honestly: *Climb as high as you can — the pressure is rising behind you.* The seed line reads "Temple seed" in ascent mode, because the number it shows is the project's, not the backdrop world's.
- **Nothing in Game changed.** The runtime needed no new engine surface: `loadChunks`, `buildAscent`, `caught()` and `heightReached()` were already public, already proven. That is the north star paying rent — a second, structurally different game shipped as a *host* change plus two lines in a data file.

## Code built

- `src/Project.hpp` / `src/Project.cpp` — `mode` and `chunksFile` on the Project struct, parsed as format-1 keys, written back only when non-default.
- `src/runtime_main.cpp` — the project-folder argument; the ascent boot (chunks loaded and climb built before the title, so the title stands at the climb's foot); `Screen::Caught` with its overlay; the height HUD and ascent title/seed lines; R restarts the same temple.
- `games/temple-run/project.myengine` — declares `mode=ascent`, `chunks=chunks.txt`.
- `tests/tests_main.cpp` — the project-mode check (suite now 55).

## Verification

- Harness: **55 passed, 0 failed** — an ascent card parses with its kind and chunks intact and round-trips through `saveProject`; a scene game's saved card contains no `mode=` or `chunks=` line; the shipped Temple Run card loads as an ascent when the suite runs from the repo root.
- The boot itself, from the runtime's own log: *Project loaded: Temple Run (format 1, seed 20261) · Chunks loaded from games/temple-run/chunks.txt: 5 definitions · Ascent built: seed 20261, 5 chunks (Gauntlet, Steps, Islands, Steps, Zigzag), top at y=150* — one command, `MyRuntime games/temple-run`, and the second game stands up.
- Screenshot: `docs/shots/lesson-76-temple-run-title.png` — the actual runtime window on the headless rig, showing TEMPLE RUN and its subtitle over the dimmed climb. One rig note, stated plainly: bare Xvfb has no window manager and parks windows half off-screen, so the capture is the visible fragment of the title screen; on a real desktop the window sits whole. The flow past the title (Enter → climb → caught → R) is verified by the state machine's symmetry with Gold Rush's proven frame and by every Game-level check beneath it; the keys themselves are Zac's to press in the playtest.

## Explain-it check

_(Post-handover format: why are mode/chunks additive keys instead of a format-2 bump? Why does Temple Run's R re-deal the same seed while Gold Rush's deals a random one? Why did this lesson need no changes inside Game? — the answers live in the Concept section above.)_

## Next step

Lesson 77: packaging — Temple Run as a standalone folder (runtime binary, its project, its assets) that runs without the repo, and the version moves to 1.5: the engine that ships two games.
