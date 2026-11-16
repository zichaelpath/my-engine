# Devlog 074 — The Rising Pressure

## Goal

Give the climb its teeth: the pressure that rises behind the player, the height score it races against, and the end of the run when it catches him. Temple Run becomes a game you can lose.

## Concept

- **One line is the whole mechanic.** The pressure is a world y. Everything below it is consumed; it starts below the world (y=1200), waits out a grace delay (2.5 s — time to take the first jump, not to sightsee), then climbs at a constant 26 units/second. Both numbers are public fields next to gravity and jump speed, tuned the same way: by playing, later from a panel if they earn one.
- **Height is the score, banked at the feet.** The score is the highest the player's *feet* have ever been above the floor — measured at the hitbox's bottom, the part of him the pressure is hunting. It is a max, not a current value: a fall costs you time against the line, never score. Air counts — the apex of a jump banks its height the frame it happens, which rewards the full, committed jump over the cautious hop.
- **Caught is the win's mirror.** When the line reaches his feet the run ends the way Gold Rush's win does: input zeroes, a hit-stop beat and a trauma shudder announce it (the Feel system's grammar, reused not reinvented), the result is logged with height and time, and the world stays alive behind the outcome. The pressure itself freezes — the story is over; the line has what it came for.
- **It only exists in the ascent.** `buildAscent` arms the pressure; `regenerateWorld` disarms it. Gold Rush has no line, no timer, no caught state — the harness asserts a fresh world stays pressure-free forever. A game mechanic that leaks into every other game on the engine would be an engine smell; this one is a run state, scoped to the run that built it.
- **The band is drawn, the fantasy is not.** Render draws a glowing band from the line to the world's floor with a bright leading edge — under the actors, over the terrain. What it *is* — lava, flood, rising dark, the temple's wrath — is Zac's naming decision, still open; the engine's job is only to make it climb and glow. The colour is a constant until the fantasy lands and recolours it.

## Code built

- `src/Game.hpp` — the pressure's public face: `pressureActive()` / `caught()` / `pressureY()` / `heightReached()`, tuning fields `pressureSpeed` / `pressureDelay`, and the private run state.
- `src/Game.cpp` — `buildAscent` arms it; `regenerateWorld` disarms it; `update()` runs it (grace, rise, height banking, the catch with hit-stop + trauma) and zeroes input when caught; `render()` draws the band and its leading edge.
- `tests/tests_main.cpp` — the rising-pressure check (suite now 53).

## Verification

- Harness: **53 passed, 0 failed** — a fresh Gold Rush world shows no pressure after two seconds of updates; the idle climber's line reads exactly 1200 at t=2 s (grace), ≈1178 at t=3.33 s (on schedule at 26 u/s), and catches him between frames 240–300 with height banked at exactly 0 and the line frozen afterwards; a jumping climber banks a full jump's height (85–108) and *keeps* it after the catch.
- Probe (shipped chunks, project seed): the idle player's line reads **1200 at 1 s, 1161 at 4 s**, and the catch lands at **frame 266 — 4.43 s**, the line exactly at his feet (1150), logged by the engine itself.
- Screenshot: `docs/shots/lesson-74-rising-pressure.png` — three fixed-camera moments: grace (no band), the rise (orange under the floor), the catch (the band at full width, at his feet).

## Explain-it check

_(Post-handover format: why is the score measured at the player's feet rather than his head or centre? Why does the pressure freeze when it catches him instead of climbing on? Why does regenerateWorld disarm the pressure instead of buildAscent being the only place that touches it? — the answers live in the Concept section above.)_

## Next step

Lesson 75: the endless deal — as the player climbs past the dealt chunks, new ones are dealt above and consumed ones are removed below, so the climb (and the run) never has to end at a ceiling.
