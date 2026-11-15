# Devlog 033 — The Frame: Title, Pause, Win (Arc 7 Continues)

## Goal

Give the Runtime the frame every real game has: a title screen, a pause, and a win screen — as an explicit state machine instead of scattered flags.

## Concept

- **Four screens, one variable.** `enum class Screen { Title, Playing, Paused, Won }` in the Runtime. Every frame asks two questions of it: should the simulation advance (only in Playing), and what should be drawn over the world (the HUD everywhere but Title; an overlay per screen). The transitions are few enough to list: Enter starts, P/Esc pauses, P resumes, the win flag promotes Playing → Won, R in Won deals a new world and starts it.
- **What a key means depends on the screen — that is the machine's whole job.** Esc used to quit the Runtime unconditionally; now Esc pauses during play, quits from Title/Pause/Won, and P shares the pause duty. Same keys, contextual meanings, no global "paused" boolean drifting out of sync with a "title" boolean — one variable holds the truth.
- **The simulation never runs behind the title.** Because `update` is gated on Playing, the title's backdrop is a *frozen* world — and, more importantly, the run timer cannot start before the player does. A speedrun clock that starts at program launch is a bug this design makes impossible.
- **Presentation stays in the host.** `Game` didn't change at all this lesson: it already exposed `won()`, `runTime()`, the counters, and `newRun`. The state machine is the Runtime's opinion about *how a session flows*, and the editor — which has its own session model (play-test, Lesson 28) — is untouched. Two hosts, two frames, one game.

## Code built

`src/runtime_main.cpp` only: the Screen enum and its transitions; title overlay (game name, dynamic goal line, seed line), pause overlay, win overlay (banner + final time + restart prompt); dim backdrop for overlays; HUD suppressed on Title; Esc's meaning made state-dependent.

## What broke

The first attempt at verifying the Won screen wasted ~15 minutes: I copied the repo to /tmp and ran a fresh CMake configure for a throwaway build — two configures ended up running concurrently in the same build directory (the first had been backgrounded, not finished), corrupting each other, and one died inside SFML's own find_package. The fix was to stop being clever: temporarily patch the dot count in the *real* tree, rebuild incrementally (7 seconds), verify, revert, rebuild. `git status` proves the shipped tree never contained the tweak. Lesson recorded: on this rig, a temporary in-tree patch plus a fast incremental build beats a parallel throwaway tree every time.

## Verification

Every transition exercised live in the Runtime, by screenshot: **Title** (GOLD RUSH, the dynamic goal line, "World seed: 12345", dimmed frozen world, no HUD) → Enter → **Playing** (HUD live, timer counting from 0:01) → P → **Paused** (overlay, HUD frozen at 0:02) → P → Playing. The **Won** screen was verified with the temporary 3-dot build described above: the world was swept quadrant by quadrant, gold dots detected programmatically in the screenshots, and the player keyboard-driven onto each — the HUD climbed 0/3 → 1/3 → 2/3 → 3/3, whereupon the Won overlay appeared with the final time frozen at 1:40 in both the banner and the HUD. Pressing **R** dealt a visibly different world and reset the HUD to 0/3, 0:01 — the full loop, closed. The tweak was then reverted and both hosts rebuilt clean; the real game still has 40 dots.

## Explain-it check

_(Post-handover format: why one enum instead of separate title/paused/won booleans? Why is the timer-fairness property a consequence of gating update, rather than a special case? Why does none of this live in Game? — the answers live in the Concept section above.)_

## Next step

Lesson 34 — Arc 7 closes: package the game. A release build of the Runtime with its assets, a versioned zip Zac can hand to anyone, and the README turned into the game's front page — the engine's first shippable artifact.
