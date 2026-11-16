# Devlog 092 — Sky & Atmosphere: The World Gets a Horizon

## Goal

Every frame until today cleared to a near-black void, and the heightfield ended at a hard edge against nothing. Give the world an atmosphere: a gradient sky overhead and distance fog that dissolves the far edge into it.

## Concept

- **The sky is a dome the camera lives inside.** A UV-sphere `Mesh` (radius 1, positions only — the Lesson 91 class, reused the day after it landed) is drawn scaled 260× around the camera's current position, first in the frame, with depth writes off: everything else draws over it, and because its model matrix re-centres it on the eye every frame, you can fly forever and never reach it. Its shader does no lighting at all — the object-space position *is* the view direction, and the fragment colours by height: zenith blue above, pale horizon at eye level, grey ground haze below.
- **Fog is a handover, not an effect.** Both lit programs now end the same way: mix the lit colour toward the fog colour by a linear factor over distance (near 55, far 140). The trick that makes it read as *atmosphere* rather than a filter is agreement: the fog colour, the sky's horizon colour and the frame's clear colour are one constant, so the terrain's far edge, the fog and the sky all meet at exactly the same shade — the world dissolves into the sky instead of ending in front of it.
- **The sky is never fogged, and the fog never touches the near field.** The sky program has no fog uniforms (it is the thing fog fades *into*); the fog factor clamps at 0 inside 55 units, so the playable foreground stays exactly as lit and shadowed as Lessons 89–90 left it.
- **A small catch worth naming:** the specular code had `normalize(camera − fragment)` in hand, and fog needs the *unnormalized* vector's length — so both shaders now keep the raw vector and its length, and derive the direction from them. The normalized vector alone has thrown the distance away.

## Code built

- `src/gl_main.cpp` — the sky program (gradient by direction height); `buildSkyDome()`; the atmosphere palette (one horizon colour shared three ways); fog uniforms in both lit programs with the mix applied after lighting and shadow; the clear colour becomes the horizon; the dome drawn first each frame with `glDepthMask` off/on around it.
- No loader growth (`glDepthMask` is a 1.1 direct call), no harness change — this lesson is all GPU-side, and its evidence is the frame.

## Verification

- All eight shaders (four programs) compile; the host renders under Mesa; harness unchanged at **67 passed, 0 failed**.
- The frame (`docs/shots/lesson-92-sky-fog.png`): the void is gone — a blue gradient overhead (measured (169, 187, 209) near the frame top, between the zenith and horizon constants, exactly right for this slightly downward-looking camera), paling to the horizon where the terrain's far edge now fades out instead of stopping. Compare any earlier lesson shot: the same world, finally sitting *in* somewhere.

## Explain-it check

_(Post-handover format: why is the sky drawn with depth writes off, and why does its model matrix follow the camera? Why must the fog colour and the horizon colour be the same constant? Why did the shaders need the unnormalized view vector? — the answers live in the Concept section above.)_

## Next step

Lesson 93 (stretch): the editor grows a 3D viewport — the GL heightfield rendered into the editor's own window, so the two halves of the engine finally share a screen.
