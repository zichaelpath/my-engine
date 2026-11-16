# Devlog 086 — Play-Test Travel: The Test Plays the Journey

## Goal

The editor's play-test (Lesson 28) played one scene in isolation: walk through a door or touch the Goal and nothing answered, because travel lived only in the Runtime. An author testing a campaign had to export and run the game to feel its flow. Make the play-test play the journey: doors and Goals work in the editor exactly as they do in the shipped game — and Stop still puts the author's scene back untouched.

## Concept

- **The editor borrows the Runtime's answers, it doesn't invent new ones.** During a test, the same latches the Runtime polls (`pendingExit`, `goalTouched`) are polled by the editor, and answered with the same moves: load the named scene, land on `Spawn:<from>`; a Goal completes the level and travels to the next scene in the project's list, or reports the campaign complete at the last one. One definition of how travel works, two hosts that honour it — the same discipline as the load ritual (Lesson 64) and the widget kit (Lesson 80).
- **The test has its own geography.** The editor's *current scene* never changes during a test — a separate `testScene` tracks where the test is. This matters because the panels (Levels, Project) keep describing the file the author opened, while the Console narrates the journey the test is on. Conflating them would make Save write the wrong scene mid-test.
- **Stop must undo travel, so the snapshot grew a memory.** Lesson 28's snapshot held entities and the clock — enough when a test couldn't leave its scene. A travelling test needs three more facts: the starting scene's path, the painted tiles, and the world's recipe (seed + noise frequency). Stop restores in layers: reload the starting scene's *file* when it has one (or regenerate its recipe when the start was untitled), then reimpose the snapshot — entities, tiles, clock — so even unsaved author edits come back exactly. The test remains what Lesson 28 promised: experiment freely, keep nothing.
- **Travel is logged, not silent.** Every door and Goal the test answers writes a Console line ("Play test: door to …", "Play test: Goal reached in …", "…campaign complete"), because in a test the *point* is learning whether the journey works — the log is the test report.

## Code built

- `src/main.cpp` — `sceneBaseOf` helper; the scene-aware test snapshot (tiles, start scene, seed, noise); the layered Stop restore; the "TESTING in <scene>" status line; and the post-update travel block polling the goal/exit latches during tests, answering them with the Runtime's moves.

## Verification

- Harness: **63 passed, 0 failed** — unchanged count by design: every mechanism the travel block calls (the latches, loadScene, placePlayerAtSpawn, the campaign list) is already pinned by the Arc 7 checks, including check 60's full journey; this lesson is editor wiring over verified machinery, and inventing a duplicate check would prove nothing new.
- Editor smoke: the editor boots the Two Rooms project cleanly with the new code (screenshot: `docs/shots/lesson-86-playtest-travel.png`, the Game View fragment this rig can capture). The click-path itself — Play, walk to the Goal, arrive in room 2, Stop, be back in room 1 untouched — joins the playtest gate's hands-on list, stated plainly rather than simulated.

## Explain-it check

_(Post-handover format: why does the test track its own scene instead of changing the editor's current scene? Why does Stop restore in layers — file first, snapshot second — instead of just the snapshot? Why must the snapshot remember the seed for an untitled start? — the answers live in the Concept section above.)_

## Next step

Lesson 87: the second viewport — two cameras on one world, the roadmap's stretch goal, attempted for real. Lesson 88 closes Arc 8 at MyEngine 1.7.
