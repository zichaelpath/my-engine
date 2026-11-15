# Devlog 024 — The Inspector (Editing the Living Scene)

## Goal

An Inspector panel that shows the selected entity's properties *and edits them* — name, position, size, colour, solid — writing straight into the live game while it runs.

## Concept

- **The inspector is a view with a pen.** Lessons 22–23 built selection as shared state with read-only readers. The inspector is the first *writer* panel: every widget binds directly to the entity's fields, so a drag on Position moves the crate in the world the same frame, and renaming updates the Hierarchy row above it (watch "Crate / Crate" become distinguishable — last lesson's small accusation, answered). `Game` grew exactly one thing: a mutable `entities()` overload for tools. The runtime never calls it.
- **Text input needs a truce with the frame loop.** You can't bind a string field directly to a text box that re-renders 60 times a second while someone types. The standard truce: a small edit buffer that *syncs in* when the selection changes and *writes back* on every change event. Selection switches re-sync; typing never fights the game.
- **Honest limits, on the record.** Colour edits are real data (they save and load) but currently render only on sprite-less entities — every entity in today's scene has a sprite, and sprite tinting is a deliberate non-goal this lesson, so the colour widget's effect is invisible until a plain entity spawns. The sprite field itself is shown read-only: swapping a sprite must also swap its hitbox (Lesson 19's coupling), and that belongs in a proper "change sprite" operation, not a raw number field. Player-controlled is likewise displayed, not editable — the game keeps a dedicated player index, and pretending otherwise would be a footgun with a UI.
- **Editing a running simulation is the point.** The game doesn't pause for the inspector: drag a crate into the player's path and the next step collides; untick Solid and the player walks through. That's what separates an editor from a level *file* with a preview.

## Code built

- `Game`: mutable `entities()` overload (tools-only; the runtime is unaffected).
- Editor: the Inspector panel — name (buffered InputText), position and size (drag fields), colour (0–1 floats ↔ bytes, the Lesson 3 conversion in its third costume), solid checkbox, read-only sprite/player-controlled info, and a "nothing selected" hint when the selection is empty.

## What broke

Nothing in the build. The first scripted rename arrived as "Lake Cra" — synthetic typing at the default speed outran the editor's very low frame rate under Xvfb and dropped its last characters. Slower typing (150 ms per character) delivered the full string; a rig artefact, recorded so future-me doesn't chase it as an InputText bug.

## Verification

Built clean on the Linux rig. The Inspector opened on the selected crate with every field correct (position 560/380, size 40×40, colour 220/140/60, solid, sprite 1). The rename test then proved the write path end-to-end in a single frame (`docs/shots/lesson-24-editor.png`): the Inspector's name field reads "Lake Crate", the Hierarchy row updated live to match, the debug panel reads "Selected: Lake Crate (560, 380)", the crate keeps its outline — and an F5 save put `entity=Lake Crate|560|380|…` in `scene.txt`, so the edit lives in the game's data, not just in a panel. The empty state also verified: with nothing selected the panel shows its hint text instead of stale fields.

## Explain-it check

_(Post-handover format: why does the name field need a buffer instead of binding to the string? Which inspector edits are visible immediately in today's scene, which one isn't, and why? Why is the sprite shown read-only? — the answers live in the Concept section above.)_

## Next step

Lesson 25 — drag to move: grab the selected entity in the Game View and drag it, the inspector's position fields following along (the first gizmo, in embryo).
