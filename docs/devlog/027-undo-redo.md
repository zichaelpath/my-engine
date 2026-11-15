# Devlog 027 — Undo/Redo (Snapshots, Not Regrets)

## Goal

Ctrl+Z / Ctrl+Y (and Undo/Redo buttons) for editor actions: drags, spawns, deletes, and Inspector edits all become reversible.

## Concept

- **Snapshots beat commands at this scale.** The command pattern stores each action's inverse (move: from→to; delete: the corpse + its index) — precise, but every action type needs bespoke undo logic, and continuous edits (a drag, a colour slide) fight it. The snapshot approach stores one thing: *the whole entity list before the action*. Entities are small structs and the list is short, so a snapshot costs almost nothing and covers every edit path uniformly — including ones not built yet. The devlog names the trade honestly: at ten thousand entities this becomes the command pattern's job; at thirty, uniformity wins.
- **The snapshot is taken BEFORE, at commit points.** One per gesture, never per frame: at a drag's grab, before a spawn, before a delete that will actually happen (a refused player-delete leaves no history), and in the Inspector on widget *activation* — the press frame, before the gesture's first change lands. A checkbox flips during its own call, so its pre-state is reconstructed and pushed explicitly; the exception that proves the rule is understood.
- **Restoring must repair identity.** Undo assigns the saved list back through `Game::replaceEntities`, which re-derives the player index from the `playerControlled` flag — the flag is the identity, so a restored list can never point the game at the wrong entity. The editor's selection clears on undo/redo: indices may now mean different entities, and Lesson 26's discipline applies to time travel too.
- **Scope, stated:** undo covers *entity edits*. World generation (seed, reshuffles) is not in the history — regenerating the terrain doesn't clear the stacks, and undoing won't un-regenerate a world. Scene loads clear both stacks: history belongs to the scene it happened in. One known wart, also stated: a gesture that changes nothing (clicking into the name field and leaving) still leaves a snapshot, so an undo can appear to do nothing once. Cheap at this scale; the command pattern is where that wart dies.

## Code built

- `Game`: `playerIndex()` getter; `replaceEntities(list)` with flag-derived player repair.
- Editor: undo/redo stacks (100-snapshot cap) with push/undo/redo operations; Ctrl+Z, Ctrl+Y and Ctrl+Shift+Z shortcuts gated away from text input; Undo/Redo buttons with stack counts in the Hierarchy; snapshot hooks at the drag grab, spawn, delete, and every Inspector widget; both scene-load paths clear the history.

## What broke

Nothing — clean build, and the full verification sequence passed first try.

## Verification

Built clean on the Linux rig. One continuous editor session exercised the whole cycle, and both the console and the saved file agree with the script: drag the crate to (662.7, 437.0) → **Ctrl+Z** ("Undo (3 entities)") → **Ctrl+Y** ("Redo (3 entities)") → **Spawn crate** (4 entities) → **Ctrl+Z** ("Undo (3 entities)") → F5. The saved `scene.txt` holds exactly the expected end state — three entities with the crate at the *redone* drag position (662.66, 437.03) — and the Hierarchy footer reads "1 back / 1 fwd", matching a hand-trace of the two stacks through the whole sequence. Undo of a drag, redo of it, and undo of a spawn: all proven against the file, not just the screen.

## Explain-it check

_(Post-handover format: why snapshot before the action rather than after? Why does restoring go through a function that re-derives the player instead of a raw assignment? Why is world generation outside the history, and what clears the stacks? — the answers live in the Concept section above.)_

## Next step

Lesson 28 — play-in-editor with restore: pressing Play snapshots the scene, and stopping returns every entity to where it was — the editor becomes a place you can experiment in without fear. (Arc 5's closer.)
