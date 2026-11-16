# Devlog 089 — Specular Light: Water Learns to Shine

## Goal

Arc 9 opens where its roadmap said it would: MyGL's flat Lambert sun gains Blinn-Phong specular highlights, with materials — water shines first and hardest, grass barely answers. Zac's parallel LearnOpenGL study covers this same ground; the engine now speaks it too.

## Concept

- **A highlight is a mirror test.** Lambert asks "how squarely does this face meet the sun?" Specular asks a second question: "does this face mirror the sun into the camera?" Blinn-Phong answers it with the *halfway vector* — the direction exactly between sun and view: where the surface normal aligns with it, the face is a perfect mirror for that fragment and the highlight lands. The exponent (shininess) controls how forgiving the alignment is: a high exponent is a polished mirror (tight, bright), a low one is brushed metal or skin (broad, faint).
- **Specular needs two things diffuse never did: the fragment's world position and the camera's.** That is why this lesson's real work is plumbing — a `uModel` matrix joins `uMVP` in both programs, world positions become varyings, and the camera position becomes a per-frame uniform. A material is then just numbers: the terrain carries its (shininess, strength) per-vertex, assigned by tile type at build time (DeepWater 90/0.95, Water 64/0.85, Sand 10/0.20, Grass 6/0.12, Forest 8/0.15, Rock 20/0.30); the crates take theirs as uniforms (varnished wood: 24/0.30).
- **The loader grows by use, as always.** `glUniform1f` did not exist in the engine until a shader needed a float uniform — GLLoader gained exactly one pointer and one load line, the standing rule since Lesson 35.
- **Optics is not a bug.** From the default camera, the flat lakes show almost no highlight — the mirror zone for a high sun sits behind that viewpoint, which the math predicts and the probe confirms. Specular is view-dependent by definition; fly the camera to a mirror angle and the water lights up.

## Code built

- `src/gl_main.cpp` — both shader programs: `uModel` + `uCameraPos` plumbing, world-position varyings, the Blinn-Phong term; the terrain vertex layout grows a specular pair (eleven floats), assigned per tile by the new `tileSpec`; crate material uniforms.
- `src/GLLoader.hpp` / `src/GLLoader.cpp` — `glUniform1f`, loaded like everything else.

## Verification

- The real binary: all four shaders compile, every new uniform resolves (the host exits if one is missing — none were), terrain and crates build and render under Mesa (screenshot: `docs/shots/lesson-89-specular.png`, the heightfield from the default camera).
- The highlight itself, measured: a probe renders the terrain program's own shaders over flat quads at the exact mirror geometry. Water centre: **(251, 255, 255)** — the highlight saturates. Grass at the same geometry: **(84, 163, 84)** — pure Lambert, no lift. Water off-mirror (corner): **(38, 78, 164)** — the predicted Lambert blue. Material difference and view dependence, both proven in numbers.
- Harness: **64 passed, 0 failed** (the 2D game is untouched; the loader change is additive).

## Explain-it check

_(Post-handover format: what does the halfway vector represent, and why does a bigger shininess exponent make a smaller, brighter highlight? Why did specular force a model matrix into shaders that had survived on MVP alone? Why is there no highlight on the lakes from the default camera — and what would you do to see one? — the answers live in the Concept section above.)_

## Next step

Lesson 90: shadows — a depth pass from the sun, and terrain that shades itself.
