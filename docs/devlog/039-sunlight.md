# Devlog 039 — Sunlight: Normals and a Directional Light

## Goal

Light the heightfield for real: per-face normals in the mesh, a directional sun in the fragment shader — and delete Lesson 38's fake (the hand-darkened skirts) now that the real thing replaces it.

## Concept

- **A normal is a face's answer to "which way do you point?"** Lighting needs it per fragment; this mesh gets it per vertex, and because every face here is axis-aligned, the builder knows each normal *exactly* — tops (0,1,0), skirts along their axis — no cross products required. (The general method, for arbitrary triangles, is the cross of two edges; this builder would only rediscover what it already knows.) The vertex format grows to match: position + normal + colour, nine floats, attribute locations 0/1/2 — the shader's `layout` qualifiers and the `glVertexAttribPointer` offsets are two halves of one contract, and they changed together.
- **Lambert's law is one dot product.** `diffuse = max(dot(normal, sunDirection), 0)` — a face square to the sun takes full light; edge-on takes none; turned away clamps at zero instead of going negative. An ambient floor (0.35) keeps unlit faces readable instead of black: `light = 0.35 + 0.65 × diffuse`. That single line is the entire lighting model — directional light, no distance falloff, no specular yet — and it's the same equation every fancier model is built from.
- **The sun is a direction, not a place.** `uSunDir` points *toward* the sun — low in the west, slightly south, normalized once on the CPU with the engine's own `normalized()`. Directional light models a sun effectively at infinity: every face with the same normal gets the same light, anywhere in the world, which is why the terraces shade so consistently across the whole map.
- **Normals ride along unrotated — for now, legitimately.** The terrain's model matrix is the identity, so world normals are object normals. The day a *rotated* model carries normals, they will need the inverse-transpose of its rotation; that lesson is noted, not dodged.
- **The loader census: +1** (`glUniform3f`, the sun's channel).

## Code built

- `src/gl_main.cpp` — shaders with normal attribute + Lambert fragment lighting; `buildTerrain` emits exact normals and no longer darkens skirts; the sun uniform; the 9-float vertex layout.
- `src/GLLoader.*` — `glUniform3f`.

## What broke

Nothing. Build clean, both uniforms located (a miss would have failed the run before the first frame, by the Lesson 36 design).

## Verification

Same start viewpoint as Lesson 38, before and after: the flat colour chart is gone. In the high shot the whole world sits under one consistent sun; in the low shot the proof is per-block — sand cubes in the lake show a lit face and an ambient face, rock outcrops read as solid blocks with a bright side and a dark side, and the shading directions agree across the entire map (west faces lit, east faces at the ambient floor), which is exactly the signature of a directional light and impossible to fake with per-tile tricks. Triangle count unchanged (18,604): lighting cost us vertex bytes, not geometry.

## Explain-it check

_(Post-handover format: why does the dot product measure "how square to the sun" a face is? Why is there no distance term? What breaks about normals the day the terrain model starts rotating, and what's the fix called? — the answers live in the Concept section above.)_

## Next step

Arc 8's last piece: texture. The crate texture joins the 3D world (UVs, a GL texture object, sampling in the shader), and Gold Rush's crates stand in the terrain they belong to. Then Arc 9: project format, packaging on Windows, and the release.
