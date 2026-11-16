# Devlog 054 — A Second Way to Move: Gravity & Grounded

## Goal

Arc 3 (Platformer Physics) opens by giving entities a **second movement model**. Gold Rush's top-down steering stays exactly as it was; beside it, a platformer model where input owns X, gravity owns Y, and the entity carries a fact the top-down game never needed: **grounded**. This is the foundation the vertical Temple Run climbs on.

## Concept

- **Models are per-entity data, not engine modes.** `Entity::moveModel` (`TopDown` / `Platformer`) plus `velocity` and `grounded` fields. A world can mix them — a platformer player dodging top-down drifters is a legal world — because the model is a property of the thing moving, not a switch the whole engine flips.
- **One collision truth.** The platformer integrates velocity into a desired move, then hands it to the *same* axis-separated `resolveCollisions` the top-down model has used since Lesson 15 — entities, terrain blocks, and the world clamp all answer both games. New physics that forks collision is two bugs waiting to disagree; this arc refuses to fork it.
- **Grounded is earned every frame.** Gravity always asks (velocity.y grows by 1800·dt, capped at a 900 terminal), the ground always answers (the resolver or the clamp allows less travel than was asked), and the comparison — asked vs allowed, the same arithmetic the bump detector uses — is the grounded fact. Standing still is not a state you enter; it is a question re-answered 60 times a second, which is why walking off a ledge needs no special case.
- **A landing is not a bump.** The bump sound/trauma now excludes platformer Y-blocks: landing is a different event with a different feel, and it gets its own treatment in Lesson 55 instead of borrowing the wall's.

## Code built

- `src/Entity.hpp` — `MoveModel` enum; `moveModel`, `velocity`, `grounded` fields.
- `src/Game.*` — physics numbers as public data (`gravity` 1800, `terminalVelocity` 900, `platformerSpeed` 230); the platformer branch in the movement system (velocity integration feeding the shared mover); grounded/head-bonk bookkeeping after the world clamp; the bump exclusion.
- `tests/tests_main.cpp` — two new checks (suite now 28).
- Persistence note: `moveModel` is in-memory only this lesson; scene files don't carry it yet. It joins the format with entity types in Arc 5, deliberately — one format change, versioned once.

## What the tests caught

The first fall test placed the player *inside a terrain block's column*: the resolver, which assumes you arrived from your movement side, politely placed him **on top** of the block he was embedded in (y 100 → 50.9, grounded). The physics was doing its documented job; the test's premise was wrong. The test now finds a clear drop column empirically — reject any column whose first frames don't move down — which is also a fair warning, recorded here: **spawn points must not overlap solids**, because the resolver will interpret the overlap as an arrival.

## Verification

- Harness: **28 passed, 0 failed.** A platformer player in a clear column falls (y grows), lands (grounded), zeroes his fall velocity, and rests at a bit-exact stable height across further frames; a second fall lands on the crate entity — entities are ground too.
- Probe strip: dropped at y=30 above the crate → y=52.5 and accelerating at 9 frames → **y=100.0, grounded, velocity 0** — exactly the predicted rest (crate top 150 − hitbox offset 28 − hitbox height 22).
- Screenshot: `docs/shots/lesson-54-fall.png` — the three instants of that fall.

## Explain-it check

_(Post-handover format: why is grounded recomputed every frame instead of set on landing and cleared on jumping? Why reuse the top-down resolver instead of writing platformer collision? Why did the embedded spawn end up on top of the block? — the answers live in the Concept section above.)_

## Next step

Lesson 55: jumping with its full feel kit — and the input seam that lets tests (and later, scripts and AI) drive a player without a keyboard: coyote time, jump buffering, variable jump height, landing recovery.
