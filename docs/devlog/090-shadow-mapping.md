# Devlog 090 — Shadows: The Terrain Shades Itself

## Goal

The roadmap calls shadow mapping "the single biggest visual step left in the 3D host," and it is right: until now the heightfield's steps were lit by their normals alone, so a cliff and the ground it towers over took the same sun. Render the scene once from the sun, keep only depth, and let every fragment ask whether anything stands between it and the light.

## Concept

- **A shadow is a depth comparison in the sun's space.** Pass 1 draws the world from the sun's viewpoint into a depth texture (the *shadow map*) — for each texel, the distance to the nearest surface the sun can see. Pass 2 draws the scene normally; each fragment is projected into that same light space, and if its depth there is greater than the stored depth, something nearer the sun is in the way: the fragment is shadowed, and it keeps only ambient light (diffuse *and* specular are direct light — both are cut).
- **The sun's camera is a box, not a cone.** Sunlight is parallel, so its projection is orthographic — which is why `Mat4` gained `orthographic` this lesson (harness-pinned: the box's corners map to the NDC cube). The box sits 70 units out along the sun direction, ±60 across, near 1 / far 160; the matrix is computed once, because this sun never moves.
- **Two classic artifacts, two standard answers.** *Acne* — a surface shadowing itself in stripes, because stored depth and fragment depth disagree at grazing angles — is answered with a slope-scaled bias (more bias where the face turns away from the sun). *Hard, aliased edges* are answered with PCF: nine taps around the fragment's texel, averaged, so shadow borders land soft. Fragments outside the sun's box sample a border depth of 1.0 (the farthest possible) and read as lit — the failure mode is "no shadow," never "false shadow."
- **The loader grew by five, minus one.** Framebuffer objects needed `glGenFramebuffers`, `glBindFramebuffer`, `glFramebufferTexture2D`, `glCheckFramebufferStatus` — and `glActiveTexture`, which turned out to be declared by this platform's `<GL/gl.h>` already (1.3 lives in the header here), so it was removed the same day and the header now records why. Verify against the actual headers; never assume — the standing rule pays again.

## Code built

- `src/Mat4.hpp` — `Mat4::orthographic`.
- `src/GLLoader.hpp/.cpp` — the framebuffer entry points.
- `src/gl_main.cpp` — the depth-only program; the 2048×2048 shadow framebuffer (depth texture, `GL_NONE` colour, border depth 1.0); the per-frame two-pass flow (sun pass, then the lit pass with the map on texture unit 1); the shadow test written into both fragment shaders (PCF 3×3, slope-scaled bias), cutting diffuse and specular together.
- `tests/tests_main.cpp` — the orthographic check (suite now 65).

## Verification

- All six shaders compile; the framebuffer reports complete; the host renders under Mesa.
- Directional proof, not vibes: the L90 frame against the L89 frame from the identical camera — **2,481 terrain pixels darkened by >10, exactly 0 brightened**, the rest unchanged. Shadows only subtract light, and ~6% of the visible terrain (the lee of the rock steps, the ground east of the crates) now sits in it. The screenshot (`docs/shots/lesson-90-shadows.png`) shows the field's new depth: plateaus read as plateaus.
- Harness: **65 passed, 0 failed.**

## Explain-it check

_(Post-handover format: why does the sun use an orthographic camera while the player uses a perspective one? What is shadow acne, and why does the bias grow on grazing faces? Why does a shadowed fragment lose its specular highlight too? — the answers live in the Concept section above.)_

## Next step

Lesson 91: meshes as assets — a Mesh class owning its VAO/VBO/EBO properly, and an OBJ loader, so the crate stops being a hand-typed array.
