# Devlog 080 — Shared UI Widgets: Screens Built from Parts

## Goal

Arc 7's third lesson pays the debt Lesson 79 deliberately took on: the runtime's screens were hand-rolled per-screen text blocks, and the campaign's level select was plain text lines. Build the shared widget kit — Label, Button, Panel as engine objects — and rebuild every runtime screen from it, mouse included.

## Concept

- **Three widgets cover every screen the engine has.** `Label` is a line of text that knows how to centre itself; `Panel` is a backing rectangle (the overlay dim is one); `Button` is a stateful, clickable row. Title, pause, win, caught, campaign-complete and the level select are all compositions of these three — so a style change (a colour, an outline, a hover brightness) now lands on every screen at once, which is the entire reason shared UI exists.
- **Widgets own geometry and drawing; hosts own input.** A Button knows its rect, its states, and how it looks in each; it does not know what a click *means*. The host feeds it the pointer (`setHovered`), asks `clickable(point)` when a click lands, and acts. That split is what keeps the kit reusable by the next host — a future game's menu, a different screen entirely — without dragging the runtime's state machine into GameLib.
- **State is drawn, not described.** A Button's four situations are four looks: resting (slate), hovered (brighter), selected (the gold ring — keyboard focus made visible), disabled (dimmed, grey text — a locked level). The level select needed no new widget for its locked/done/selected rows; it needed a Button honest about state, plus text that says `[done]` or `(locked)`.
- **Keyboard and mouse are two routings to one action.** The refactor's structural move: each screen's *actions* (start play, new run, climb again, campaign again) were lifted into lambdas the keyboard switch and the new mouse routing both call. Before, "what Enter does" lived inside a switch case; a mouse path would have duplicated it and the two would have drifted. Now they cannot — there is one `startPlay`.
- **Behaviour didn't change; construction did.** Every screen keeps its layout positions, texts and keys. The Play button is new capability (mouse users can start), as are Resume/New world/Climb again/Play it again/Title buttons — each mirroring an existing key, none replacing one. The editor is untouched: ImGui is its UI system, and this kit is for shipped-game screens.

## Code built

- `src/Ui.hpp` / `src/Ui.cpp` (new, in GameLib, namespace `ui`) — `Label` (house text style, `centreOn`), `Button` (rect/state/hover/selected/disabled, `placeCentred`, `contains`/`clickable`, state-coloured draw), `Panel` (rect/fill/outline draw).
- `src/runtime_main.cpp` — every `sf::Text` and the `makeText`/`centreOn` helpers deleted; all screens composed from widgets; screen actions extracted to lambdas; mouse routing added (clicks ask the visible screen's buttons; level rows select, Play starts); hover fed per frame; the campaign score line gets its own Label (it previously borrowed the Caught screen's text object — a wart the refactor surfaced and fixed).
- `tests/tests_main.cpp` — the widget check (suite now 58).

## Verification

- Harness: **58 passed, 0 failed** — `placeCentred` geometry exact, `contains`/`clickable` hit-testing, the disabled gate (geometry yes, clickable no), a Panel carrying its rect, hover measurably brightening a rendered Button's fill (pixel-sampled clear of the text), and a Label's bounds centring where `centreOn` puts them.
- Probe: the kit composed as the Two Rooms title screen — dim Panel, title/subtitle/prompt/seed Labels, three level Buttons in done/selected/locked states, a hovered Play — rendered to `docs/shots/lesson-80-ui-widgets.png`. It is the runtime's real arrangement, drawn by the same calls.
- The runtime builds clean with the old text code fully removed (no `sf::Text` remains in it) — the refactor is a replacement, not a layer.

## Explain-it check

_(Post-handover format: why do widgets not handle their own clicks? Why were the screen actions lifted into lambdas before mouse support was added, instead of the mouse switch duplicating the key cases? Why is "selected" a Button state rather than the host drawing a rectangle around a row? — the answers live in the Concept section above.)_

## Next step

Lesson 81: save files — a versioned `save.dat` in the project folder persisting campaign progress, Gold Rush best times and per-player settings, loaded at boot and written at the moments progress happens.
