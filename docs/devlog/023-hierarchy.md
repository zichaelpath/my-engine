# Devlog 023 — The Hierarchy Panel (A Second Door Into Selection)

## Goal

List every entity in a Hierarchy panel, and let clicking a name select it — the same selection state the Game View picking writes. Small lesson, deliberate point.

## Concept

- **Selection is a model, not a mouse trick.** Lesson 22 could have been read as "the editor highlights what you click." This lesson proves the deeper structure: one `selected` variable, two independent writers (the world view and the list), and every reader (highlight, readout, future inspector) indifferent to which door you came through. That's the shape all editor state should have — panels are views onto shared state, never owners of private copies.
- **The list tells truths the view hides.** In the world, the two crates look like "a crate here, a crate there." In the hierarchy they're two rows with the *same name* — "Crate", "Crate" — which is exactly the kind of scene-hygiene problem lists exist to expose. (ImGui handles the duplicates with a `##index` suffix: the visible label stays clean while the widget ID stays unique.) Names become editable in the inspector lesson, which is when this stops being cosmetic.
- **Order is information.** The list runs in spawn order, which is also draw order (bottom to top) and index order — three views of one fact. When entities gain parents later in the engine's life, this flat list becomes a tree; the selection contract won't change.

## Code built

Editor only again (`src/main.cpp`): a Hierarchy panel — entity count, one `Selectable` row per entity (unique-ID labels), clicks writing the shared `selected` index and logging like the picking path does. No changes to `Game`, picking, or the highlight: the new panel plugged into the existing state and everything downstream (outline, readout) lit up for free — the lesson's thesis, demonstrated by its own diff.

## What broke

Only the test rig: the first scripted clicks on the hierarchy rows kept missing, because the editor rewrites `imgui.ini` on exit — reusing one run directory across attempts let the panel drift between the aim screenshot and the click. A fresh directory plus coordinates read from a screenshot of the *same* run landed first try. (Filed under screenshot-rig lore: never aim from a previous run's geometry.)

## Verification

Built clean on the Linux rig. `docs/shots/lesson-23-editor.png` shows the whole thesis in one frame: the second "Crate" row highlighted in the Hierarchy, the yellow selection outline around that crate in the Game View, "Selected: Crate (560, 380)" in the debug panel, and "Selected: Crate (index 2)" in the console — all driven by a list click, with the highlight and readout code untouched since Lesson 22. The duplicate "Crate" names sit side by side in the list exactly as predicted, waiting for the inspector to make them editable.

## Explain-it check

_(Post-handover format: what does it prove that the highlight lit up for hierarchy clicks without the highlight code changing? Why do the two "Crate" rows not confuse ImGui? What three orderings coincide in the list today? — the answers live in the Concept section above.)_

## Next step

Lesson 24 — the Inspector: the selected entity's real properties (name, position, size, colour, solid, sprite) shown *and editable*, writing back into the live game.
