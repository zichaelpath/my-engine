# Devlog 032 — The Gold Rush: A Real Game (Arc 7 Opens)

## Goal

Turn the engine into a game: the gold dots the decor generator has scattered since Lesson 10 become collectible, with a counter, a run timer, a win state, a Runtime HUD, and R for a fresh world.

## Concept

- **The game was hiding in the decor.** Forty gold dots, a character who walks, terrain that slows him, sounds for everything — the only missing pieces were *stakes*: a count, a clock, and an end. Collection is a distance check from the player's feet (the hitbox centre) to each dot's centre, run at the end of `update`; the run timer accumulates until the last dot falls, then freezes — your time is your score. Winning is a state (`m_won`), not an event the hosts have to catch, so the editor and the Runtime both see it.
- **The rules live in `Game`; the hosts just present them.** The Runtime draws a HUD (gold count, clock) and a win banner; the editor shows the same numbers in its debug panel. One simulation, two presentations — the split from Lesson 9 paying off again. Restart is `Game::newRun(seed)`: the Runtime's R key hands it a random seed, and regeneration resets the run (counter, timer, win flag) because a regenerated world *is* a fresh run.
- **Text needed a font, so the engine bundles one.** DejaVu Sans (freely licensed; the licence file ships beside it in `assets/`) loads once in the Runtime; if it's missing, the game plays on with the HUD disabled and one error logged — garnish is never a dependency, same as music. HUD text is drawn in *screen space* (the default view, after the world) so it never moves with the camera.
- **Two more synthesized sounds:** the pickup (a two-note rise whose pitch climbs slightly as the count grows, in an 8-step cycle) and the win arpeggio — the soundscape stays 100% generated.

## Code built

- `Game`: gold state (collected/total/won/run time), collection + win in `update`, `newRun`, run reset inside `regenerateWorld`, total set by `generateDecor`.
- Runtime: bundled-font HUD (counter + clock), win banner, R-to-restart when won.
- Editor: Gold and Run time readouts in the debug panel.
- `assets/DejaVuSans.ttf` + licence; `pickup.wav` and `win.wav` from the tools script.

## What broke

Nothing in the build. The verification had one honest near-miss: two dots survived the first collection pass by a few pixels — the collection point is the *feet* centre, ~14 world units below the sprite centre my drags were aiming with, and the smallest dots (reach 19) fell just outside it. Aiming the feet, not the sprite, finished the world.

## Verification

Verified end-to-end in the running editor, at full-world zoom: gold dots were detected in the screenshot programmatically and the player was drag-teleported onto each one (the editor's own drag tool — collection is position-based, so a dropped player collects on the next frame). **Gold climbed 0 → 38 → 40 / 40**, the panel declared "40 / 40 - CLEARED", `win.wav` loaded, and the console logged **"World cleared in 134.913223 s - all 40 gold collected"** — the run timer then stayed frozen at 134.9 s while the game clock kept counting, proving the freeze. The Runtime screenshot shows the HUD live ("Gold: 0 / 40, Time: 0:06") over the world. The 134.9 s "score" includes long analysis pauses between drags; a human playing properly will do much better, and that's rather the point of a timer.

## Explain-it check

_(Post-handover format: why does the win live in Game rather than in the Runtime that displays it? Why does regeneration reset the run? Why is the HUD drawn in a different view than the world? — the answers live in the Concept section above.)_

## Next step

Lesson 33 — the game gets its frame: a title screen in the Runtime (name, "press Enter", the seed of the day), a pause, and the win screen as a real state machine instead of a banner — the difference between a demo and a game.
