# Devlog 082 — Arc 7 Complete: MyEngine 1.6.0

## Goal

Close Arc 7 (Levels, Campaigns & Saves) the way Arc 6 closed: the arc's claims re-verified as one end-to-end story, the version bumped to say a milestone landed, both games repackaged and clean-room booted, and the tag cut.

## What the arc built (Lessons 78–81)

- **78 — Multi-scene projects.** The project card's `scene=` line became a repeatable level list; doors and landings became naming conventions (`Exit:<scene>`, `Spawn:<from>`), with Game latching travel and hosts doing it; spawn markers visible to authors, hidden in shipped games.
- **79 — Campaign structure.** The `Campaign` model (ordered levels, unlock-in-order, completion recomputing `current` so replays never rewind), the `Goal` finish-line latch, the runtime campaign flow, and the Two Rooms demo campaign generated through the engine's own APIs.
- **80 — Shared UI widgets.** `ui::Label` / `ui::Button` / `ui::Panel` in GameLib; every runtime screen rebuilt from them; screen actions lifted into lambdas shared by keyboard and the new mouse routing.
- **81 — Save files.** Versioned `save.dat`: campaign progress restored only when the saved level list still matches, per-seed best times, Temple best height, per-player settings (camera shake on K) — written at the moment progress happens.

## The arc-close verification

The arc's four lessons each proved their own piece; this lesson proves they compose. The new harness check (suite now **60**) plays the whole journey in the runtime's own shapes: a two-level project is built as files, level one's Goal is touched in a real `Game::update`, the completion persists through `writeSave` — and then everything is rebuilt cold. The save restores only because the level list matches, the restored campaign shows level one completed and level two in play and unlocked, and the player resumes standing exactly on level two's `Spawn:one`. Separately, the probe wrote a played save against a scratch copy of the shipped Two Rooms project and rendered its restored title screen from the restored state — room1 `[done]`, room2 selected — which is the screenshot below.

## The release

- CMake and the packaging script move to **1.6.0**; `CHANGELOG.md` gains the 1.6.0 entry.
- Both games repackage (`dist/GoldRush-1.6.0-linux.zip`, `dist/TempleRun-1.6.0-linux.zip`) and pass the clean-room boot from an empty folder: the runtime reports `MyEngine 1.6.0`, each project loads, music starts, and Temple Run builds its ascent with the canonical seed-20261 deal (Gauntlet, Steps, Islands, Steps, Zigzag).
- Tag `v1.6.0` marks this commit.

## Honest edges

- The playtest gate is unchanged and still Zac's: 1.6.0's campaign flow, widget screens and mouse routing are harness- and probe-verified, but no human has clicked Play yet — the same first-real-hardware session that covers the 1.5.0 items (audio, left-facing retest, Temple Run hands-on) now also covers Two Rooms end to end. Findings feed a 1.6.1 list.
- The editor still opens one scene at a time from a multi-scene project (the FileSystem panel lists them all; double-click loads). A dedicated campaign/levels editor view is Arc 8 territory, not a quiet omission.

## Explain-it check

_(Post-handover format: what single sentence distinguishes what Lessons 78, 79 and 81 each own — geography, progression, persistence? Why does the arc-close check rebuild everything cold instead of reusing the session's objects? — the answers live in the lessons above.)_

## Next step

Arc 8 — The Editor Grows Up, Windows and All: terrain painting as scene-stored edits, and the Game View popped out into its own window.
