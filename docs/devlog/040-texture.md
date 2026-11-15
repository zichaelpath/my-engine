# Devlog 040 — Texture: The Crate Comes Home

## Goal

Close Arc 8: a GL texture sampled in a shader, UV coordinates on a real mesh, and Gold Rush's own crates — read from the Game's entity list — standing on the terrain at their world positions.

## Concept

- **A texture is just another buffer — on the GPU, sampled by coordinate.** The CPU decodes the file (SFML's `sf::Image` does that in every host; decoding is not rendering), the pixels upload once into a texture object with `glTexImage2D`, and from then on the shader asks for colours by **UV** — (0,0) to (1,1) across the surface — instead of receiving them per vertex. Images load top-row-first and GL textures count from the bottom row, so the image is flipped on the way in; forget that and every texture stands on its head.
- **The cube grew up: 24 vertices, indexed.** Faces can no longer share vertices — each face needs its own normals *and* its own UV corners — so the cube is 4 vertices per face × 6, drawn with an index buffer (`glDrawElements`, the path Lesson 36 set aside). Vertex layout: position + normal + UV, 8 floats.
- **Two programs, one scene.** The terrain keeps its colour program; the crates get a textured sibling — same sun, same Lambert line, base colour from `texture(uTexture, vUV)`. A real engine is mostly this: many programs, one consistent light. Filtering is a choice with a look: `NEAREST` magnification keeps pixel art crisp (this crate is 32×32 pixel art), while GPU-generated mipmaps with trilinear minification keep it from shimmering at distance.
- **Placement is data conversion, not hand-tuning.** Each crate entity's 2D pixel centre becomes a tile, the tile becomes terrain space, and the crate stands on `tileHeight` of the ground beneath it. Move a crate in the 2D editor and its 3D self moves too — one world, two renderers, still.
- **Translation + uniform scale leave normals honest** (the model matrix): translation never rotates, uniform scale only stretches a normal's length, and the shader normalizes — the Lesson 39 caveat stays theoretical for now. `Mat4::scaling` joins the math header to make that model.

## Code built

- `src/gl_main.cpp` — `createProgram` factored out of the terrain setup (two programs now); `buildCrateMesh`; the textured shader pair; texture upload with flip + mipmaps; crate placements from `game.entities()`; per-crate model matrices in the draw.
- `src/GLLoader.*` — census +2 net (`glUniform1i`, `glGenerateMipmap`); see What broke for the four that *left*.
- `src/Mat4.hpp` — `scaling`.

## What broke

Two things, one of them a genuine discovery. First: I added the four texture functions to the loader and the compiler refused their PFN types — because **texture objects are OpenGL 1.1 (1997)**. `glGenTextures`, `glBindTexture`, `glTexImage2D` and `glTexParameteri` were never post-1.1 extensions; they're declared by `<GL/gl.h>` and called directly, like `glClear`. My mental model had textures filed under "modern GL" and they are older than the programmable pipeline itself. The loader carries only what's genuinely loadable; the four were removed and called directly.

Second, a rig chase: several screenshots came back as bare clear-colour frames and I bisected the draw code before proving the binary innocent — consecutive grabs of the running app showed the world, crates and all. The failures were the Xvfb capture path (grabs landing before the window was really up and positioned after a rebuild). The durable fix is procedural: confirm the window's geometry before flying, and grab twice — identical hashes mean a live frame. It's in the devlog so the next session doesn't re-chase it.

## Verification

The high shot shows both crates in the world at their 2D positions; the close shot is the proof of texture: plank grain, frame and edging all readable, right way up, sun-lit on the same terms as the terrain (bright top, shaded side), standing on the grass beside the pond exactly where the 2D game keeps it. The build log reports the upload ("Crate texture uploaded: 32x32") and the placements ("Crates placed: 2").

## Explain-it check

_(Post-handover format: why does a textured cube need 24 vertices when the coloured one needed 8? Why flip the image? Why does NEAREST suit this texture and mipmaps still matter? — the answers live in the Concept section above.)_

## Next step

Arc 8 is complete: window, shaders, matrices, camera, terrain, light, texture. Arc 9 turns the pile into a product — a real project format, the Windows package of Gold Rush built on Zac's machine, and the release checklist. The engine's 2D half also gets its due there: everything MyGL learned stays a guest in the GL host until the editor itself grows a 3D view — that is post-1.0 territory, and the roadmap says so honestly.
