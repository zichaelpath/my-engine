# Devlog 028 — Play-Test With Restore (Arc 5 Closes)

## Goal

A Play test button that snapshots the scene, and a Stop button that restores it exactly — so the editor becomes a place you can *try things* in, consequence-free.

## Concept

- **Until now the editor had no modes — the scene was always live and every change was forever** (or an undo away). Real editors separate *authoring* from *testing*: in Unity, everything you do in Play mode evaporates when you stop. This lesson adds the smallest honest version: Play test takes a snapshot (the entity list and the game clock); Stop puts both back through the same `replaceEntities` the undo system built. The player can run to the far corner, and one click returns him to spawn as if the trip never happened.
- **The snapshot is not undo.** They look alike — both store entity lists — but they answer different questions. Undo is a *history of authoring actions*; the test snapshot is a *save point for an experiment*. They don't share stacks, and neither pollutes the other: a test in the middle of an authoring session leaves the undo history exactly as it was. Conflating them is how editors get undo systems that fill up with gameplay.
- **The contract is written on the panel.** While testing, the debug panel says so: "TESTING: changes are discarded on Stop". Tools stay live during a test (dragging a crate mid-test works — and is discarded like everything else), because the guarantee is about the *restore*, not about locking the toolbox. A scene load cancels test mode: a snapshot of a scene that no longer exists is a lie, and both load paths drop it.
- **Arc 5, in one sentence:** the editor can now select, list, inspect, drag, spawn, delete, undo, and experiment — every verb an editor needs, each built on the shared-state architecture Lesson 22 started. The game itself was touched exactly twice in the whole arc (a mutable accessor and two small APIs), which is the architecture working.

## Code built

Editor only (`src/main.cpp`): play-test state (snapshot + clock + flag); Play test / Stop test controls in the debug panel with the testing notice; the old Play/Pause toggle relabelled Resume so the two ideas never share a word; scene loads cancel test mode. Restore reuses Lesson 27's `replaceEntities` (player re-derived from its flag) and rewinds `game.time`.

## What broke

Nothing — clean build, full cycle verified first try.

## Verification

Built clean on the Linux rig. The full test cycle ran in one editor session: Play test clicked at spawn (375, 275) → the player was driven to (607, 441) with the panel showing the Stop button and the "TESTING: changes are discarded on Stop" notice → Stop clicked → the player is back at (375, 275) in the panel, in the view, and in the F5-saved `scene.txt` (`entity=Player|375|275|…`), with the console narrating "Play test started - scene snapshot taken" / "Play test stopped - scene restored". The game clock visibly rewound too (19.88 s mid-test, 17.73 s after the stop despite more wall time having passed — the difference is the rewound test session). Arc 5 is complete: every claim in its five-lesson run was verified against the running editor, most of them against the scene file as a second witness.

## Explain-it check

_(Post-handover format: how is the test snapshot different from an undo snapshot, though both store the entity list? Why does the panel advertise that changes are discarded instead of locking the tools during a test? Why does loading a scene cancel test mode? — the answers live in the Concept section above.)_

## Next step

Arc 6 — audio: sound effects and music through SFML, an audio resource in the cache's family, and something in the game worth hearing (footsteps? a splash when you wade in? UI clicks in the editor?).
