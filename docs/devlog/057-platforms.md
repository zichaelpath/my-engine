# Devlog 057 — Platforms With Manners

## Goal

Arc 3's toolkit completes: **one-way platforms** (jump up through them, land on top) and **moving platforms** (patrol a path, carry their rider). Both are entity data, both are answered by the machinery already built — this lesson is mostly proof that Lessons 54–56 were shaped right.

## Concept

- **One-way is a question the resolver already knows.** A one-way platform collides only for a platformer, only in the Y pass, only moving down, and only if his previous bottom was above its top — the identical previous-position guard Lesson 56 gave tiles, applied to an entity flag. Rising through it, approaching from the side, standing under it: no collision, by the same arithmetic that decides landings everywhere else. No special case was added to the movement code; a flag was added to the *question*.
- **Platforms are kinematic, riders are carried.** A platform moves by fiat — position follows its patrol (home ↔ `platformTo` at `platformSpeed`, arriving exactly and turning around), ignoring collision entirely, and records how far it travelled this frame (`frameDelta`). Carrying is then one honest check, run before the rider's own movement: *were you grounded, and are your feet resting exactly on a platform's top?* Then your position takes its delta, and your own gravity re-earns the ground underneath you a moment later. The platform never pushes; the rider simply stands somewhere that moved.
- **What platforms deliberately are not (yet):** they don't collide with the world (a patrol path is the author's promise), they don't carry NPCs (the player is the only rider the check looks for), and their fields aren't in the scene format — like `moveModel`, they arrive with entity types in Arc 5, one versioned format change instead of three.

## Code built

- `src/Entity.hpp` — platform fields: `platform`, `oneWay`, `platformTo`, `platformSpeed`, plus patrol state and `frameDelta`.
- `src/Game.cpp` — `updatePlatforms` (patrol motion, exact arrivals, turnarounds); the carry check in `update`, before the rider moves; the one-way rule in `resolveCollisions`' entity pass.
- `tests/tests_main.cpp` — two new checks (suite now 36).

## Verification

- Harness: **36 passed, 0 failed.** One-way: from the staged perch, a held jump's feet cross at least 15 units *above* the platform's top (he passed through it), and the fall ends grounded with his feet at exactly the platform's top. Moving: a rider dropped onto a patrolling platform travels 60.0 ± 2 units in 60 frames at 60 u/s, feet glued to the deck throughout.
- Probe strip: landed at player x=36.0; 45 frames later x=81.0 — exactly 45 units of carry — still grounded.
- Screenshot: `docs/shots/lesson-57-platforms.png` — the ride, before and after.

## Explain-it check

_(Post-handover format: why does the one-way rule live in the resolver instead of the movement code? Why does carrying run before the rider's own movement rather than after? Why don't platforms collide with the world they patrol through? — the answers live in the Concept section above.)_

## Next step

Lessons 58–59 close Arc 3: the Physics panel (every number from Lessons 54–57, live in the editor, with the platform/solidity toggles) and a purpose-built climber test scene — a vertical shaft of rock, one-ways, and a moving platform, staged so Zac can feel the whole toolkit in one climb when he playtests.
