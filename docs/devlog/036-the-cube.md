# Devlog 036 — The Cube: Depth, Indices, and the MVP Chain

## Goal

Make it genuinely 3D: a spinning cube with indexed geometry, a working depth test, and the model/view/projection matrix chain computed by the engine's own new Mat4.

## Concept

- **Three spaces, three matrices.** A vertex starts in the cube's own space; the *model* matrix places (and spins) it in the world; the *view* matrix re-expresses the world from the camera (here, the world slides 2.6 units away — moving the camera and moving the world are the same matrix read backwards); the *projection* matrix squeezes the view frustum into clip space, which is where perspective — far things smaller — actually comes from. `gl_Position = projection * view * model * position`, read right to left. Mat4 is header-only, column-major (the layout GL wants, uploaded untransposed), and small enough to hold in your head: identity, multiply, three rotations, translation, perspective.
- **Indexing is sharing.** A cube drawn as raw triangles names 36 vertices; as 8 corners + 36 indices it names each corner once, and the faces *share* them. Sharing is visible in the screenshot: the corner colours flow continuously across the shared edge instead of restarting per face. The VAO now also captures the element buffer — bind order is part of the state the VAO remembers.
- **Depth is a second image.** Enabling `GL_DEPTH_TEST` (against the 24-bit depth buffer the context was created with in Lesson 35) makes the GPU keep, alongside the colour image, a per-pixel distance image; a fragment only lands if it's nearer than the one already there. That's the entire trick behind "solid": the cube's back faces are drawn (no culling yet) and lose, pixel by pixel, to its front faces.
- **The loader grew by two lines** — `glGetUniformLocation` and `glUniformMatrix4fv`, the uniform channel the MVP travels through. The census discipline from Lesson 35 holds: functions join the loader when a lesson needs them. (`glDrawElements` needed no line — it's OpenGL 1.1, linked directly.)

## Code built

- `src/Mat4.hpp` — identity/multiply/translation/rotationX/Y/Z/perspective, column-major, with an `operator*`.
- `src/gl_main.cpp` — cube geometry (8 corners coloured by coordinate), index buffer, depth test on, per-frame MVP from the clock, `glDrawElements`.
- `src/GLLoader.*` — two new entry points.

## What broke

Nothing in the build or the render. The first verification screenshot caught the cube almost face-on (a valid frame of the animation, a useless picture of it), so the shot was re-taken at an angle that shows the form — a photography problem, not a code problem, and recorded as one.

## Verification

The screenshot shows the cube mid-spin: two faces meeting at a crisp vertical edge, the RGB gradient continuous across it (shared indexed vertices, interpolating), foreshortening on both faces (perspective), and no trace of the back faces (depth test). The log confirms "uMVP located" — a missing uniform would have failed the run before the first frame, by design.

## Explain-it check

_(Post-handover format: in `projection * view * model`, which matrix touches the vertex first, and why does the order read backwards? Why does sharing vertices change how colours blend at an edge? What would the cube look like with the depth test off — which faces would wrongly appear? — the answers live in the Concept section above.)_

## Next step

Lesson 37 — a camera you can fly: lookAt joins Mat4, WASD + mouse-look (or arrow keys) move the eye through space, and a small field of cubes gives the motion something to read against. After that, Lesson 38 brings the two worlds together: the tile map, extruded into a 3D heightfield.
