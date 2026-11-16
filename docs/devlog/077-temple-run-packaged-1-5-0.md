# Devlog 077 — Temple Run, Packaged: MyEngine 1.5.0

## Goal

Close Arc 6 the way Arc 9 of Phase 1 closed: with a versioned, standalone, clean-room-verified package — this time of the *second* game — and the engine version moving to 1.5.0 to say what happened: the engine outgrew its first game.

## Concept

- **One binary, two packages.** The packaging script generalises from "package Gold Rush" to "package a game": `package_game.sh [build-dir] [goldrush|templerun]`. What differs between the two zips is only what *should* differ — the staged project card, Temple Run's extra `chunks.txt`, and the player readme. The binary, the shared asset set, and the default `engine.ini` are identical, because that is the engine thesis in filesystem form: the game is the data beside the binary.
- **1.5.0, and the 1.0.1 that folded into it.** The version lives in exactly one place (CMake's `project()`), and everything — hosts' log lines, `VERSION.txt`, the zip names — reads from it or from the script constant that mirrors it. The fixes that were waiting as a planned 1.0.1 patch (the left-facing sprite, the vendored KHR header) ship inside 1.5.0; the playtest that gated them now gates this version instead — it is, in Zac's own words from this evening, "the next working version" he downloads tonight.
- **Clean room, same standard as 1.0.0.** The zip was unpacked into an empty folder outside the repo and run there: the log reads `MyEngine 1.5.0 — Runtime`, the project loads as Temple Run, the chunks load from `./chunks.txt`, the ascent builds with the project's own deal, and the music file starts. The one thing the rig still cannot prove is sound itself — this VM has no audio device (its ALSA errors are the rig confessing, not the package failing); the first real listen remains part of Zac's playtest, as it has been since Lesson 44.
- **Arc 6, closed.** The second game arc set out to prove the north star — that a rookie could download this engine and make a *different* game with it. The evidence: Temple Run's engine-side cost was a chunk data file, five small platform mechanics inside systems that already existed, two keys in a project card, and host wiring. Its fantasy — what the climb is, what the pressure is — is still Zac's to name, and the game plays fine unnamed, which is rather the point: the engine never needed to know.

## Code built

- `CMakeLists.txt` — `project(MyEngine VERSION 1.5.0)`; every host now logs 1.5.0.
- `tools/package_game.sh` — generalised to package either game (name, project source, extra files, readme per game); VERSION 1.5.0.
- `tools/TempleRun-README.txt` — the player readme: the pitch, the controls, the one-way and moving platform truths.
- `CHANGELOG.md` — the 1.5.0 entry.
- `dist/` (gitignored) — `TempleRun-1.5.0-linux.zip` and `GoldRush-1.5.0-linux.zip`, ~2.2 MB each.

## Verification

- Harness: **55 passed, 0 failed** on the 1.5.0 tree — the version bump touched nothing behavioural.
- Clean room: unzipped into an empty folder and run standalone — version line, project, chunks, ascent and music all confirmed from the run's own log.
- Screenshot: `docs/shots/lesson-77-templerun-package.png` — the *packaged* Temple Run's title screen, running from the clean room (the rig's window-placement crop applies, as noted in Lesson 76).
- Git: annotated tag `v1.5.0` marks this lesson's commit, beside `v1.0.0` on Lesson 42's.

## Explain-it check

_(Post-handover format: why does one binary ship in both zips instead of a Temple Run-specific build? Why did the 1.0.1 patch fold into 1.5.0 rather than shipping separately? What does the clean-room run prove that running from the repo cannot? — the answers live in the Concept section above.)_

## Next step

Phase 2 continues with Arc 7 (Levels, Campaigns & Saves) — and Zac's playtest of 1.5.0 (Windows build, the left-facing re-test, the first real-hardware listen, and Temple Run in his hands) can land whenever tonight's download happens; its findings open Arc 1's remaining gate and feed the 1.5.1 list.
