# Devlog 084 — The Game View Pops Out

## Goal

Arc 8's second lesson is the roadmap's "windows and all": the editor's world leaves the dock and becomes its own OS window — movable to a second monitor — with every interaction (picking, dragging, painting, panning, zooming) working there, and the layout remembered between sessions.

## Concept

- **One render path, two display surfaces.** The world still renders exactly once, into the same `gameView` RenderTexture as always (selection highlight and editor overlays included). The pop window is a *display* of that texture plus an *input source* — not a second renderer. Two views of one world (the roadmap's flagged stretch goal) would mean two cameras and two renders; this lesson deliberately stops at one view with two homes, which is what a second monitor actually needs.
- **Input is unified before it is duplicated.** The docked panel produced its pointer facts from ImGui; the popped window produces them from SFML events and polling. Rather than fork the interaction code, both fill one `ViewPointer` (active / clicked / down / released / right-down / wheel / pixel / pan delta), and picking, dragging, painting and panning are written once against it. The interaction code cannot tell which window the hand is on — that is the design, and it is also why the refactor had to come first: Lesson 83's paint code moved into the shared path unchanged in behaviour.
- **The texture follows the home.** Docked, the RenderTexture sizes to the panel; popped, it sizes to the OS window, so the picture is 1:1 in both places and `mapPixelToCoords` keeps one meaning. Resizing either home just works, because the size was always re-derived per frame.
- **Layout is a preference, and preferences persist.** Popping or docking writes `popGameView` into engine.ini (Config grew the `set()` it always should have had — a host recording a choice, then saving). Restart the editor and the Game View is where you left it. Closing the OS window docks it back rather than stranding the view; the panel meanwhile shows where the view went and offers the dock button.
- **Focus stays simple.** Both windows live in one loop; the main window's events feed ImGui as before, the pop window's are drained separately and never reach ImGui. Neither window blocks the other, and closing the *editor* closes both.

## Code built

- `src/Config.hpp` / `src/Config.cpp` — `Config::set`: record a value in memory for the next `save()`.
- `src/main.cpp` — the pop window (`sf::RenderWindow`) and its open/close plumbing with the engine.ini preference (restored at startup); the per-frame pop event drain and mouse poll; the `ViewPointer` struct; the interaction code (paint / pick / drag / pan / zoom) extracted into one lambda fed by either source; texture sizing following the active home; the blit to the pop window; the docked panel's popped-state placeholder with its Dock button, and the Pop out button in the docked state.
- `tests/tests_main.cpp` — the Config::set round-trip check (suite now 62).

## Verification

- Harness: **62 passed, 0 failed** — including the new check that a `set()` choice survives save and reload, which is the preference mechanism the pop-out state rides on.
- Live boot, verified on screen: with `popGameView=1` in engine.ini, the editor starts popped — its log records the pop, the docked Game View panel shows the "popped out… Dock Game View" placeholder, and the separate OS window renders the Gold Rush world (terrain, water, crate, gold, the player). The screenshot is a crop of that real frame (`docs/shots/lesson-84-pop-out-game-view.png`); this rig has no window manager, so the two windows park overlapped in the corner — on a desktop they open as the two ordinary windows they are.
- Hands-on items for the playtest gate, honestly listed: dragging the pop window to a second monitor, and clicking/dragging/painting *inside* it — the input path is the same code the docked panel exercises, but no headless rig here can click an OS window.

## Explain-it check

_(Post-handover format: why is the pop window a display of the existing texture rather than a second render of the world? Why did the interaction code have to be unified into ViewPointer before the second window could exist? Why does closing the pop window dock instead of just closing? — the answers live in the Concept section above.)_

## Next step

Lesson 85: editor growth continues — a scene list for multi-scene projects in the editor (open any level of the campaign from the Project panel's context), building on Arc 7's geography.
