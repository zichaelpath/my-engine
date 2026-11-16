# Devlog 094 — Arc 9 Close: The 3D Second Season, Shipped as 1.8.0

## Goal

Close Phase 2's ninth arc the way the others closed: a capstone check that exercises the arc's pieces together, the full suite on a fresh build, both games repackaged and clean-room verified, and the version bumped — engine **1.8.0**.

## The arc, in five lessons

- **89 — Blinn-Phong specular.** Materials enter the lighting model: a (shininess, strength) pair per tile type riding in the terrain vertices, one honest material for the crates. Measured, not eyeballed: 8,567 water pixels carry a highlight ≥200 where the same pixels maxed at 145 under Lambert.
- **90 — Shadow mapping.** A 2048² depth pass from the sun through a new `Mat4::orthographic`, PCF 3×3 softening, slope-scaled bias. Proven directionally: 2,481 terrain pixels darkened against the L89 frame, exactly 0 brightened.
- **91 — Mesh + OBJ.** A move-only `Mesh` owns the GPU buffers; the crate became `assets/crate.obj`, loaded by a harness-tested parser. The refactor rendered pixel-identical (0 of 120,000).
- **92 — Sky & fog.** A gradient dome on the Mesh class, one horizon colour shared by sky, fog and clear colour; the world stops floating in a void.
- **93 — The editor's 3D View (the stretch).** TerrainMesh and GLProgram extracted to the libraries; a Viewport3D FBO panel orbits the scene being edited, kept fresh by a tile hash. Probe-verified end-to-end in a real SFML context.

## The capstone (harness check 69)

The arc's pieces, composed: the shared builder's world and the OBJ-loaded crate, projected through the host's own sun constants, must fit inside the shadow camera's orthographic box. If the world ever outgrows the box, shadows would silently stop at its edge — the check hears about it first. Green, along with the rest: **69 passed, 0 failed** on the fresh 1.8.0 build.

## Verification

- Full rebuild of every target (editor, runtime, GL host, harness, both games' staging) on the 1.8.0 tree.
- Both packages clean-room verified from empty folders: the runtime reports **MyEngine 1.8.0**, Gold Rush loads its project, Temple Run loads its chunks and builds the canonical seed-20261 ascent (top at y=150).
- An honest process note: the first packaging attempt ran 38 minutes because the packager was invoked without its build-dir argument and began a from-scratch SFML build in a stray folder. It was killed, the folder removed, and the correct invocation (`BuildLinux` first) packaged both games in 22 seconds. The lesson is recorded here rather than repeated: the packager's first argument is the build dir, and a "stuck" job deserves a `ps` before it deserves patience.

## Explain-it check

_(Post-handover format: what does each of the arc's five lessons add, and which one would you remove last? What does the capstone actually guarantee? — the answers live in the lessons themselves.)_

## Next step

Arc 10 — Ship 2.0: the public-repo decision (Zac's, and it gates the arc), a showcase pass, and the final Phase 2 release. Phase 2 then stands at ten arcs, two games, one editor, two renderers, and a harness of 69 checks that all began with a window that would not open.
