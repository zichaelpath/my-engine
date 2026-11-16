# Devlog 078 — Multi-Scene Projects: Doors and Landings

## Goal

Open Arc 7 (Levels, Campaigns & Saves): a project stops being one world. Its card lists its scenes, and the player can walk from one into another — through doors the author places by naming things, landing on spawns the author names back.

## Concept

- **The list is the card, repeated.** The project format already had `scene=`; now the line is *repeatable*. Every `scene=` line joins the project's ordered level list, the first is the start scene, and `saveProject` writes the list back line for line. A one-scene project — every project shipped so far — parses and saves exactly as it did yesterday: the extension is invisible until a second line appears. That is the additive-format discipline doing its third shift (after v2 scenes and the mode/chunks keys).
- **A door is a name.** Any entity named `Exit:<scene>` is a door to that scene — no format change, no new entity field, nothing to learn in the inspector beyond typing a name. When the player's hitbox overlaps a door, `Game` latches the target in `pendingExit()` and logs it. Game owns the *touching*; the host owns the *files*: the runtime polls the latch, loads the target through the project root, and clears up. A failed load clears the latch too — a broken door must not retry sixty times a second.
- **A landing is a name, answered.** Arrival is `placePlayerAtSpawn(fromScene)`: the scene may declare `Spawn:<scene-you-came-from>` — so a level with two neighbours can receive each at its own door — falling back to plain `Spawn`, and failing honestly (false, player unmoved) when neither exists. The player is placed stopped: arriving is a teleport, not a throw. The scene the traveller left is identified by its base name (`scenes/a.scene` → `a`), which the runtime tracks as it travels.
- **Markers are for authors.** Spawn entities render as ordinary coloured squares in the editor — an author must *see* where travellers land. Shipping hosts set `hideSpawnMarkers`, and the markers vanish from the shipped game while staying in the scene data. Exits still render in both: a door is a thing in the world, meant to be found.
- **What this is not yet.** Travel preserves nothing between scenes but the player himself — no campaign state, no inventory, no "this level is done." That is the arc's next lessons (campaign structure, then saves). Today's claim is narrower and load-bearing: the *geography* of a multi-scene game works, end to end, from data alone.

## Code built

- `src/Project.hpp` / `src/Project.cpp` — `Project::scenes`, the repeatable `scene=` parse (first line = start), and the list-faithful save.
- `src/Game.hpp` / `src/Game.cpp` — the exit latch (`pendingExit` / `clearPendingExit`, cleared by every successful scene load), the per-frame door check in `update()` (hitbox overlap, like gold), `placePlayerAtSpawn` with named-then-default resolution, and the `hideSpawnMarkers` render flag.
- `src/runtime_main.cpp` — the travel loop: track the current scene, answer a latched exit by loading the target inside the project and landing the player, hide spawn markers in the shipped game.
- `tests/tests_main.cpp` — the multi-scene check (suite now 56).

## Verification

- Harness: **56 passed, 0 failed** — a two-level card lists both scenes with the first as start and round-trips through `saveProject`; a player staged inside scene A's door latches `scenes/b.scene` after one update; loading B clears the latch; `placePlayerAtSpawn("a")` lands the player at exactly (500, 300), `placePlayerAtSpawn("nowhere")` falls back to the plain Spawn at (50, 50), and a spawn-less scene refuses (false) without moving him.
- Probe: the same journey rendered — panel A, the player standing in A's orange Exit with its magenta Spawn beside him; panel B, the player arrived in scene B *on* the Spawn:a marker (he covers it exactly; the engine's log confirms "Player arrived at Spawn:a (500, 300)").
- Screenshot: `docs/shots/lesson-78-multi-scene.png` — the author's view, markers visible; the shipped runtime hides the Spawn squares and keeps the doors.
- Rig note: the build VM was replaced again overnight and the recovery recipe (placeholder headers, library symlinks) was reapplied before any of this lesson's results were trusted — the harness binary's fresh timestamp was checked, per the standing rule, before its 56/56 was believed.

## Explain-it check

_(Post-handover format: why is a door a naming convention instead of a new entity field or scene-format version? Why does Game latch the exit instead of loading the next scene itself? Why does Spawn:<from> beat plain Spawn when both exist? — the answers live in the Concept section above.)_

## Next step

Lesson 79: campaign structure — an ordered level sequence with progress (which levels are done, which is next) that the travel system advances, and a level-select flow in the runtime to present it.
