# Devlog 022 — Selection (Arc 5 Opens: The Editor Becomes a Tool)

## Goal

Click an entity in the editor's Game View and it becomes *the selected entity* — highlighted, named in the debug panel, logged. The smallest possible editor feature, and the one every later editor feature stands on.

## Concept

- **Selection is editor state, not game state.** The runtime has no concept of a selected entity, so neither does `Game`. The index lives in the editor, the picking math lives in the editor, and even the highlight is drawn by the editor — into the same render texture, in the same camera view, right after the game renders. The game doesn't know it's being edited. That separation is what will let the runtime stay clean through the whole arc.
- **Picking is a coordinate journey.** A click arrives as *panel pixels*. It becomes a texture pixel (the image is drawn 1:1 with the texture), then a *world* position via the view the texture was rendered with (`mapPixelToCoords` — the camera math from Lesson 16, run backwards), and finally a search: entities tested topmost-first (draw order reversed — the player, drawn last, wins ties), first sprite box containing the point wins. Clicking empty ground clears the selection, as editors have done since forever.
- **You pick what you see.** The test uses the *sprite* box, not the hitbox — Lesson 19's feet box is for feet, but a click targets the visible character, hood included. Different rectangles for different questions, again.
- **Identity, for now, is the index.** Selection stores an entity index, safe because nothing despawns yet. Loading a scene replaces the list, so both load paths clear the selection — an index pointing at a *different* entity after a load would be a silent lie, and the code refuses it explicitly.

## Code built

Editor only (`src/main.cpp`) — `Game` is untouched this lesson, which is the design working:

- `pickEntity(game, worldPoint)`: topmost-first sprite-box search.
- Game View panel: left-click handling (panel pixel → world via the render texture's view), selection logged.
- Render section: the yellow highlight outline drawn as an overlay after `game.render`.
- Debug panel: "Selected: name (x, y)" readout.
- Scene loads (F9 and the button) reset the selection.

## What broke

The first verification click missed the crate by roughly one pixel of world space — the test's aim assumed a slightly wrong view width, and the pick (correctly) reported empty ground. Recomputed from the camera readout the panel itself displays, the second click landed centre-crate. A miss that precise is accidental evidence the picking bounds are exact.

## Verification

Built clean on the Linux rig, editor driven by synthetic clicks. All three paths proven: clicking the crate at (560, 380) selects it — the debug panel reads "Selected: Crate (560, 380)", the console logs "Selected: Crate (index 2)", and the crate wears the yellow outline in `docs/shots/lesson-22-editor.png`; clicking the player selects the player ("Selected: Player (375, 275)"); clicking open water clears to "Selected: none". `Game` and the runtime are untouched and unaware any of this exists — the separation held.

## Explain-it check

_(Post-handover format: why does selection live in the editor instead of in Game? Trace a click: what are its three coordinate forms, in order? Why does picking test the sprite box while collision tests the hitbox? — the answers live in the Concept section above.)_

## Next step

Lesson 23 — the Hierarchy panel: every entity listed by name; clicking the list selects too (the same selection state, a second door into it), proving selection is a model, not a mouse trick.
