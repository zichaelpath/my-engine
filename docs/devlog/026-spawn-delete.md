# Devlog 026 — Spawn & Delete (The List Can Shrink Now)

## Goal

Create entities from the editor (a "Spawn crate" button) and delete the selected one (button or Delete key) — with the game's own bookkeeping surviving both.

## Concept

- **Spawning was already built; it just needed a door.** `Game::spawnEntity` has existed since Lesson 14 and applies sprite hitboxes on the way in, so the editor's button is three lines: spawn at the camera's centre (in view by construction), with the crate's standard size, colour and sprite — and the new entity *arrives selected*, ready to be dragged to wherever it belongs. Workflows are made of such courtesies.
- **Deletion is where index identity gets honest.** Erasing element *i* from a vector shifts every later index down by one. Anything holding an index must be re-examined: the editor's `selected` (cleared on a successful delete — the index's meaning changed under it) and, inside `Game`, `m_playerEntity` (decremented when the removed entity sat before the player, so the game keeps pointing at the same traveller). This is the exact pressure that eventually argues for stable entity handles; at three-to-thirty entities, disciplined index bookkeeping is still the honest, sufficient tool — and now it's written down where the pressure lives.
- **The player is not deletable.** `removeEntity` refuses the player index with a warning: the game's update, camera and HUD all assume a player exists, and an editor that can saw off the branch every system sits on is not a tool, it's a trap. The editor's delete path handles refusal gracefully — if the entity count didn't drop, the selection stays and the console's warning explains why.
- **The Delete key minds the text fields.** It only fires when ImGui says no text input is active (`WantTextInput`), so pressing Delete while renaming an entity edits the *name*, not the scene. Keyboard shortcuts that ignore focus context are how editors eat people's work.

## Code built

- `Game::removeEntity(index)`: refuses out-of-range and the player; logs; erases; repairs `m_playerEntity`.
- Editor: a shared `deleteSelected` used by the Hierarchy's "Delete selected" button and the focus-aware Delete key; "Spawn crate" in the Hierarchy, spawning at the camera centre through the existing `spawnEntity` (hitbox included) and selecting the newcomer.

## What broke

Nothing in the build (one compile fix: `main.cpp` needed `SpriteSheet.hpp` for the sprite constants — they live there, not in `Game.hpp`). The verification clicks selected crate index 2 where index 1 was intended — a one-row aim miss that the console log itself documented, and which changed nothing about what was proven.

## Verification

Built clean on the Linux rig, all four behaviours exercised in one editor session (`docs/shots/lesson-26-editor.png` shows the end state): **Spawn** — "Spawn crate" produced a fourth entity at (380, 280), the camera centre minus half-size, arriving selected with the Inspector confirming index 3, size 40×40, sprite 1. **Refusal** — selecting the Player and pressing Delete logged the amber warning "Refused to remove the player entity" and the count held at 4. **Delete** — a crate delete logged "Entity removed: Crate (index 2)", the count dropped to 3, and the selection cleared itself. **Persistence** — an F5 save wrote exactly those 3 entities, the spawned crate among them. The `m_playerEntity` repair path wasn't reachable in this scene (the player sits at index 0 and can't be removed or preceded) — it stands as defensive code for hand-authored scenes, and the devlog says so rather than claiming a test that didn't happen.

## Explain-it check

_(Post-handover format: after deleting entity 1 of 4, what does the old index 3 mean, and who had to be told? Why does the editor clear the selection on delete instead of selecting a neighbour? Why is the player undeletable, and how does the UI stay honest when a delete is refused? — the answers live in the Concept section above.)_

## Next step

Lesson 27 — undo/redo for editor actions: a small command history over the operations the editor can now perform (move, spawn, delete, property edits), Ctrl+Z / Ctrl+Y.
