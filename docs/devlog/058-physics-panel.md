# Devlog 058 — Physics You Can Touch

## Goal

Arc 2 ended by making feel *data with a panel* (Lesson 53); Arc 3's physics gets the same treatment. Every number Lessons 54–57 introduced becomes a live editor control, the solidity table becomes checkboxes, and the player's movement model becomes a switch — so the whole platformer toolkit can be *felt* in the editor, not just verified in the harness.

## Concept

- **The panel writes the same fields the harness pins.** Gravity, terminal velocity, run speed, jump speed, coyote time, jump buffer, jump cut — these were public data from the day they were born (Lesson 54), precisely so this panel would be a set of sliders over real state, not a parallel tuning file that drifts from the game. Double gravity in the panel and the jump's rise halves (v²/2g) — the harness check for this lesson retunes the field directly and measures exactly that.
- **Solidity as checkboxes closes Lesson 56's loop.** The table was always data; now it's *touchable* data. Uncheck Rock and the grid stops being ground — the harness proves the switch by dropping a platformer through a rock tile he previously landed on. A game whose geology you can re-decide live is a game whose levels can be prototyped by feel.
- **The model switch is the playtest key.** "Player is a platformer" flips one entity's `moveModel` — and suddenly Gold Rush's world is a physics playground: the same terrain reads as ground plane or as stone depending on the switch, which is Lesson 56's two-readings idea made into a single checkbox. Lesson 59's climber scene depends on this switch existing.

## Code built

- `src/main.cpp` — the Physics window: seven sliders over `Game`'s physics fields, six solidity checkboxes through `tileSolid`/`setTileSolid`, the player model switch, and grounded/velocity readouts.
- `tests/tests_main.cpp` — one new check for the levers the panel drives (suite now 37).

## Verification

- Harness: **37 passed, 0 failed** — doubled gravity measurably lowers the jump apex; rock switched off lets the drop fall through.
- The editor itself: run headlessly, ImGui's saved layout records `[Window][Physics] Pos=60,60 Size=233,395` — the panel exists, sized by its contents.
- Screenshot: `docs/shots/lesson-58-physics-panel.png` — the panel, captured from the running editor.

## Explain-it check

_(Post-handover format: why do the sliders write Game's fields directly instead of a panel-side copy that applies on a button press? Why is the model switch per-entity rather than a game-wide mode? — the answers live in the Concept section above.)_

## Next step

Lesson 59 closes Arc 3: `Game::buildClimberTest()` stages a vertical shaft — floor, two one-way ledges, a patrolling platform, a goal — and an editor button beside the Physics panel builds it, so the whole toolkit can be climbed in one go.
