# Devlog 059 — The Climber: Arc 3, Climbed End to End

## Goal

Close Arc 3 with a stage that uses everything in it: `Game::buildClimberTest()` builds a vertical shaft — a floor, two one-way ledges, a patrolling platform, a goal — and the harness *climbs it*, driving the player through the input seam from floor to goal. The editor's Physics panel gets the button that stages it, so the same climb is one click away in Zac's playtest.

## Concept

- **Staging verifies its own stage.** The first version placed the shaft at fixed coordinates — and the harness's first hop died on a rock tile two rows above the start that nobody had put there on purpose; the generated world had. So `buildClimberTest` now scans candidate origins for a *clear vertical corridor* (no solid tiles, no terrain blocks, no solid entities in the shaft's rectangle) and builds at the first one that fits — in the default world, x=1120. A test scene that only works in one world state isn't a test scene; this one adapts, and says where it landed in the log.
- **The climb is the specification.** Every hop is inside the jump's ~102-unit rise; the goal hop is loosened to 85. The driver that climbs it is a small state machine per phase, and writing it taught three lessons the engine will keep: a *level-held* jump press never produces a second edge (the first driver jumped zero times in 2,400 frames — the press must be released and re-pressed, and never released mid-rise or the cut bites); boarding a moving platform is a *timing window measured in units* (jump too close and the deck is a ceiling, too far and it never arrives — the working band is a deck-centre distance of 55–80, approaching); and the goal, hanging over the deck's path, punishes a straight jump with an underside bonk — the working play is a drift-jump from beside its span, triggered off the *leading hitbox edge*, 26–60 units out.
- **What the shaft proves, in one run:** gravity and grounded (54), the full jump kit (55), tile collision avoided and respected (56), one-way pass-throughs used twice, a moving platform boarded and ridden (57), all with the panel's numbers untouched (58). That is what "toolkit" means: the pieces compose.

## Code built

- `src/Game.*` — `buildClimberTest()`: corridor scan, five staged platform entities (rebuildable — staging twice replaces, never duplicates), player placed at the base as a platformer.
- `src/main.cpp` — "Build climber test" button in the Physics panel.
- `tests/tests_main.cpp` — the scripted end-to-end climb (suite now 38).

## Verification

- Harness: **38 passed, 0 failed** — floor (feet 1100) → Ledge1 (1010) → Ledge2 (920) → ride deck (830) → goal (745), every landing exact, the whole climb driven through `setInput`.
- Screenshot: `docs/shots/lesson-59-climber.png` — the staged shaft, player at its base.

## Explain-it check

_(Post-handover format: why does the staging scan a corridor instead of trusting fixed coordinates? Why must a scripted jump press be a state machine rather than a held level? Why is the goal approached from beside its span instead of from the deck passing under it? — the answers live in the Concept section above.)_

## Arc 3 closes

Platformer physics is complete: two movement models on one collision truth, a forgiving jump, solid terrain, platforms with manners, live-tunable numbers, and a climb that composes them all. **Next: Arc 4 — The Project Is a Place.** The north star's first pillar: a project becomes a folder the editor shows and manages — a file-system panel, file dialogs, and asset import that copies files into the project under project-relative paths.
