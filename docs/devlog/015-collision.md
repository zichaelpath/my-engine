# Devlog 015 — AABB Collision (The World Becomes Solid)

## Goal

Make things solid. The player stops at crates and terrain blocks instead of ghosting through, using axis-aligned bounding boxes (AABB) with axis-separated resolution — the movement model that gives wall-sliding for free.

## Concept

Every collider in this world is a rectangle aligned to the axes, so collision starts with one question, answered by `math::rectsOverlap`: do these two boxes share any area? (Touching edges do *not* count — that detail is what lets a mover rest exactly against a wall without the wall constantly re-triggering.)

The resolution model matters more than the test:

- **Move one axis, then resolve it.** The frame's movement is applied to X first; if the mover now overlaps anything, it is pushed back along X to exact contact. Then the same for Y. Because the axes are independent, pushing diagonally into a wall kills only the into-wall component — the along-wall component survives, and the player *slides*. The naive alternative (move both axes, then push out along the smaller penetration) sticks, snags on corners, and tunnels diagonally.
- **Colliders are a set, not a type.** The mover tests against every solid entity *and* every terrain block from Lesson 10 — two different storage systems (a `vector<Entity>`, a `vector<RectangleShape>`) feeding one resolution function through the same (position, size) shape. `Entity` gains a `solid` flag, so a future ghost or trigger zone opts out by data, not by code path.
- **Known limits, stated:** the test is discrete (position sampled once per frame). At 200 px/s and 60 FPS a step is ~3 px against 40 px colliders — tunnelling is impossible at these speeds. A 2,000 px/s projectile would need a swept test; that lesson belongs to the projectile. Resolution is also single-pass, so a mover wedged into a corner of two colliders resolves against each in turn — adequate here, and the failure mode (a pixel of overlap for a frame) is visible and benign.

## Code built

- `Entity`: new `solid` field (default true).
- `Math.hpp`: new `rectsOverlap` — the AABB test joins the toolbox.
- `Game::update`: movement is now axis-separated — apply X, resolve X, apply Y, resolve Y, then the world-bounds clamp from before.
- `Game::resolveCollisions` (new private system): pushes the mover to exact contact against all solid entities (skipping itself) and all terrain blocks, along the axis that moved.

## What broke

The build broke loudly and misleadingly: the first compile failed with a wall of errors inside the C++ standard library headers (`stl_tree.h`, `map` — "expected unqualified-id before 'namespace'"). The actual fault was in this project: adding the `solid` field to `Entity.hpp` dropped the struct's closing `};`, so every header included after it parsed as nonsense and the STL took the blame. Lesson worth keeping: when the standard library "breaks", the bug is almost always in the last header you touched, upstream of the first error. One restored `};` later, clean build.

## Verification

Built clean on the Linux rig after the fix. The editor was then *driven*: automated key input held W for 0.6 s (player up from y=275 to y≈153), then A — and the screenshot (`docs/shots/lesson-15-editor.png`) shows the result both ways it can be shown. Visually, the green player rests flush against the right face of the orange crate at (200, 150). Numerically, the debug panel reads **Player: (240, 153)** — x=240 is exactly the crate's right edge (200 + 40), the contact plane the resolver computes. Without collision the same input would have carried the player to x≈95, straight through the crate. Axis-separated sliding is exercised by the same shot: the Y movement completed fully while X was blocked.

## Explain-it check

_(Post-handover format: why resolve one axis at a time instead of both together? Why do touching edges not count as overlap? Which two storage systems feed the same collision function here, and what made that possible? — the answers live in the Concept section above.)_

## Next step

Lesson 16 — a real camera: the view stops being "whatever fits the canvas" and becomes a position in the world, with pan and zoom in the editor and follow in the runtime — the world gets bigger than the screen for the first time.
