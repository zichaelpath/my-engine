# Devlog 049 — Easing & Tweens: The Player Pops In

## Goal

Arc 2 (Game Feel) opens with its foundation: an easing library and a tween type in GameLib, proven the way this arc will prove everything — on the shipped game. Gold Rush's player now *arrives*: every new run, he pops in from nothing over a third of a second, overshooting slightly before settling. Small effect, large point: the engine can now animate *values over time along curves*, which is the primitive under half of all game feel.

## Concept

- **Easing is vocabulary.** `src/Easing.hpp` holds pure functions t∈[0,1] → progress: `linear`, `inQuad`, `outQuad`, `inOutQuad`, `outCubic`, `outBack` (overshoots above 1 mid-flight — the pop), `outBounce` (the landing). Seven curves, not the whole easings.net catalogue: each is here because a feel effect in this arc's plan speaks it. Every curve satisfies f(0)=0 and f(1)=1 *exactly* — a curve that lands at 0.999 makes every animation it drives end slightly wrong, forever.
- **A tween is one float with a schedule.** `FloatTween` (from, to, duration, curve) advances by dt, clamps at the end with no drift, and treats a zero duration as "already there" rather than a future division by zero. Game owns one for the spawn pop; the pattern generalises to any float the engine will ever animate — positions, scales, volumes, camera trauma (Lesson 51).
- **The pop is anchored at the feet.** Render scales the player's picture around his centre-bottom: same boots, eased size. He grows out of the ground he stands on instead of inflating from a corner — anchoring is the difference between a pop and a glitch. Collision is untouched: the hitbox is the full box from the first frame (Lesson 44's lesson — the picture may play, the body stays honest).
- **Feel, measured.** The harness checks the curves' endpoints, the overshoot, monotonicity, and the tween's clamp; the integration check reads `Game::playerSpawnScale()` after `newRun` (must be mid-pop) and after a second of updates (must be exactly 1). A render probe then *shows* it: scale 0.549 at 0.05 s — matching the outBack formula's own prediction — beside the settled frame.

## Code built

- `src/Easing.hpp` — the seven curves, header-only.
- `src/Tween.hpp` — `FloatTween`.
- `src/Game.*` — `m_spawnPop`, started by `newRun` (0 → 1, 0.35 s, `ease::outBack`), advanced in `update`, applied in `render` to the player's draw position/size (feet-anchored); `playerSpawnScale()` exposed for tests.
- `tests/tests_main.cpp` — five new checks (suite now 15).

## What broke

The build rig, again, not the engine: the VM was replaced mid-lesson for the second time today, wiping the system packages restored an hour earlier (xdotool, dev symlinks, headers). The rig was rebuilt from the standing recipe (versioned-lib symlinks + old-mtime placeholder headers + Xvfb) and the full build re-verified green. Recorded here because it will happen again: on this machine, "the environment forgot everything" is a known weather pattern, and the recovery is now a two-minute drill rather than a diagnosis.

## Verification

- Harness: **15 passed, 0 failed** — the five new checks (curve endpoints exact, outBack overshoots at t=0.5, outQuad monotonic, tween from/midpoint/clamped-settle, new-run pop settles at exactly 1) beside the ten from Lesson 45.
- Render probe through the engine's own instruments: `playerSpawnScale()` reads **0.549** at 0.05 s into the run and **1.000** after settling — the formula's prediction to three decimals.
- Screenshot: `docs/shots/lesson-49-pop.png` — the probe's two renders side by side: mid-pop (left, ~55% size, feet planted) and settled (right).
- Full build: all targets green; CI will re-verify on both platforms on push (Lesson 46's machine doing its job — the harness it runs now includes these checks).

## Explain-it check

_(Post-handover format: why must an easing curve land on exactly 1 rather than 0.999? Why is the pop anchored at the feet instead of the centre or the corner? Why does the hitbox ignore the pop? — the answers live in the Concept section above.)_

## Next step

Lesson 50: particles — a CPU pool in Game, emitters as data, and the splash that has had a sound since Lesson 29 finally getting its picture.
