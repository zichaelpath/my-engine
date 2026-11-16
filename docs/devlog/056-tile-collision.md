# Devlog 056 — The Ground Is Data: Tile Collision

## Goal

Until now a platformer could only stand on entities, terrain blocks, and the world's floor. This lesson makes the **tile grid itself** collision — with a solidity table deciding which tiles are matter — so terrain can be ground, walls, and ceilings. It is the last piece Arc 3 needs before platforms (Lesson 57) and a real climber's stage (Lessons 58–59).

## Concept

- **One grid, two readings.** The same rock tile is scenery in Gold Rush and stone in a platformer, and both readings are correct — because solidity is a question the *movement model* asks, not a property the world imposes. The top-down model never runs the tile pass (its world is a ground plane with walking speeds); the platformer model does. Gold Rush cannot regress: the harness proves its player still crosses rock at full stride.
- **A table, not a rule.** `tileSolid(id)` reads a six-entry table (only Rock ships solid). It looks trivially small because it is — the point is the *shape*: geology as data a project can re-decide, destined to become per-project data with entity types in Arc 5, edited in a panel after that.
- **The previous-position guard.** The first cut had a classic platformer bug: stand beside a taller rock column and gravity's tiny per-frame move made the Y-resolver "land" you *on top of it* — a step-up teleport. The fix is one question: **was your edge on the tile's side of its surface last frame?** A Y push only counts as a landing if your previous bottom was above the tile's top. Side overlap is the X pass's business, and the X pass handles it as a wall — which the tests verify to the exact face coordinate.
- **Overlapping colliders resolve in pass order** (entities, then blocks, then tiles; last push wins). A crate sitting inside rock tiles is two truths about one space; the engine's answer is deterministic but the *level design* answer is "don't overlap solids" — recorded here as the standing rule the editor will eventually warn about.

## Code built

- `src/Game.hpp` — `tileSolid` / `setTileSolid` over a data table (Rock solid).
- `src/Game.cpp` — the tile pass in `resolveCollisions`: only for platformer movers, only the handful of tiles the hitbox overlaps, answered by the same `pushOut` as everything else, with the previous-position guard on the Y axis.
- `tests/tests_main.cpp` — three new checks, plus staging helpers the whole physics suite now shares (suite at 34).

## What the tests taught (the honest part)

Turning the grid solid changed the geology under the older tests: falls that used to end on a crate now met rock first, and a jump staged near the world's top rows had its apex eaten by the world ceiling. The physics was right each time; the *stages* were unverified. The suite now stages its perches the way the engine verifies everything — empirically: scan for a patch of verified-clear tiles, move the crate there, drop, and accept the stage only if the rest height is exactly the predicted one. The wall check's step pattern doesn't exist in the default seed's world at all, so it scans a handful of seeds and runs in whichever world actually has the step — the physics claim is world-independent, and the test says so.

## Verification

- Harness: **34 passed, 0 failed.** A platformer lands on a rock tile at exactly its top (probe: tile (19,25), top 500 → rest y 450.00); a rock step stops a walking player at exactly its face; the top-down player crosses the same rock unimpeded.
- Screenshot: `docs/shots/lesson-56-rock.png` — the player standing on the generated world's own rock.

## Explain-it check

_(Post-handover format: why does the top-down model skip the tile pass instead of the table marking rock non-solid for everyone? Why does the landing guard ask about last frame's position rather than this frame's overlap depth? Why do the tests stage perches empirically instead of trusting fixed coordinates? — the answers live in the Concept section above.)_

## Next step

Lesson 57: platforms with manners — one-way platforms you jump up through and land on, and moving platforms that carry their passengers.
