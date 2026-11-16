# Devlog 085 — The Levels Panel: Geography You Can Edit

## Goal

Arc 7 gave projects a level list and Arc 8's editor could open any scene file — but the *list itself* (which scenes are in the game, in what order) was only editable by hand-editing the project card's text. Give the editor a Levels panel: the declared scenes in campaign order, openable, reorderable, removable, extendable — persisted to the card the moment they change.

## Concept

- **The list is the campaign.** The panel treats `project.scenes` as what Lesson 79 proved it is: the game's level sequence, where position means something (first = start, order = unlock order). Reordering here is not tidying files; it is editing the game's structure, and the panel says so in its own footer.
- **Every change writes the card at once.** There is no Apply button and no dirty state: swap, remove and add each re-sync `startScene` to the first entry and call `saveProject` immediately. The alternative — an in-editor list that might diverge from the file the Runtime reads — is exactly the class of bug the Project API exists to prevent. The file is the truth; the panel is a view that writes through.
- **Opening rides the one load ritual.** A row's Open button calls the same `loadSceneIntoEditor` that F9, the file browser and Project-panel double-clicks use — procgen fields re-sync, selection and undo history of the old scene are discarded — and marks the scene current, which the row then shows with its `>` marker. A new way to open a scene must not become a second way scenes load.
- **Removing a level removes a line, not a file.** The X drops the scene from the list; the scene *file* stays in the project, unlisted — the same distinction the Runtime makes (a campaign is a list over files, not the files themselves). Deleting files remains the file browser's business, with its own confirmations.
- **The panel knows when it has nothing to say.** No project open: one sentence. A project with no declared scenes: one sentence, plus Add-current when a scene is open. Editor panels that explain their empty states age better than panels that just look broken.

## Code built

- `src/main.cpp` — the Levels panel: rows in list order with current-scene marker, Open (through `loadSceneIntoEditor`), ^/v reorder, X remove (with the loop-safe break after mutation), Add-current-scene when eligible, and the `syncAndSave` write-through (start re-sync + `saveProject`) shared by all mutations.
- `tests/tests_main.cpp` — the panel's persistence contract as a harness check (suite now 63): the panel's exact operations (swap + start re-sync + save; erase + save) round-trip through a real project card.

## Verification

- Harness: **63 passed, 0 failed** — reorder and removal, with the start scene re-synced each time, persist through `saveProject`/`loadProjectFolder` exactly as the panel performs them.
- Editor smoke: the editor boots with the Levels window registered in its layout (imgui.ini), and boots the real Two Rooms project (its card loads with both scenes declared). The screenshot (`docs/shots/lesson-85-levels-panel.png`) shows that session's Game View — this rig's window parking keeps the Levels panel itself outside the capturable fragment, so the panel's rows join the playtest gate's hands-on list alongside the other GUI paths; its data path is the harness-verified one above.

## Explain-it check

_(Post-handover format: why does every mutation save immediately instead of marking the project dirty? Why does removing a level not delete its file? Why must the start scene be re-synced on every change rather than stored independently? — the answers live in the Concept section above.)_

## Next step

Lesson 86: the editor learns the campaign's shape — play-testing from a chosen level (test the campaign the way the Runtime plays it), or viewport two: a second camera on the same world. The arc closes at Lesson 88 with MyEngine 1.7.
