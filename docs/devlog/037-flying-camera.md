# Devlog 037 — Flying: lookAt and a Field of Cubes

## Goal

Give the 3D host a camera you can fly — WASD + Q/E movement, arrow-key look — built on a real `lookAt`, and a scene worth flying through: 25 spinning cubes from one VAO.

## Concept

- **A view matrix is a camera transform turned inside out.** `lookAt(eye, center, up)` builds the camera's basis from geometry: forward = normalize(center − eye); side = normalize(cross(forward, world-up)); true-up = cross(side, forward). The view matrix then rotates the world into that basis and translates by the negated eye, projected onto the basis vectors (the −dot products in the last column). There is no "camera object" in OpenGL — there is only this matrix, recomputed per frame from position + yaw + pitch.
- **Yaw and pitch are the state; the forward vector is derived.** The derivation uses the engine's own rotation conventions (yaw about Y, then pitch about X, both from Mat4), so the keys, the movement, and the matrix can never disagree about which way is forward. W flies *along* the look direction (pitch included — a true fly camera, not a walk camera), and pitch is clamped just short of vertical, where the basis math degenerates.
- **One VAO, 25 draws.** Every cube in the field is the same geometry; only the model matrix changes (grid position, then a spin with a per-cube phase so the field doesn't move in lockstep). This is the shape of all real rendering: a small number of meshes, a large number of transforms — and the reason the MVP is a per-draw uniform rather than a per-scene one.
- **Vec3 joined Mat4's header** — dot, cross, normalized, and the arithmetic the basis construction needs. cross() is the lesson inside the lesson: two perpendiculars from two known directions is how every camera, normal, and tangent frame in 3D gets built.

## Code built

- `src/Mat4.hpp` — `Vec3` (+ dot/cross/normalized) and `Mat4::lookAt`.
- `src/gl_main.cpp` — camera state and controls (WASD/QE/arrows, dt-timed), the 5×5 cube field, final camera position logged on exit.

## What broke

Nothing. The first field screenshot doubles as the check that the grid's depth ordering is right (near cubes occlude far ones across the whole field, not just on one cube).

## Verification

Two screenshots bracket a flight: the field from the start position (0, 1.4, 2.5), then the same field after 1.6 s of W and a right turn — the camera inside the grid, cubes looming, composition unrecognisable. The exit log gives the instrument reading to match: **"Camera stopped at (0.000, 0.676, −2.289)"** — 4.8 units forward along the pitched forward vector (hence the descent from 1.4 to 0.68), zero lateral drift, exactly the path the inputs describe.

## Explain-it check

_(Post-handover format: why is the view matrix an inverse rather than a placement? Why does W descend when you pitch down, and which line of code makes that happen? Why clamp the pitch — what breaks at exactly vertical? — the answers live in the Concept section above.)_

## Next step

Lesson 38 — the merger: the 2D tile world becomes a 3D heightfield. MyGL will build a mesh from the same seed and tile data as Gold Rush — water low, rock high — and the fly camera goes sightseeing over a world the player already knows from above.
