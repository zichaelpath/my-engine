# Devlog 091 — Meshes as Assets: The Crate Is a File Now

## Goal

Until today, every mesh in the GL host was a loose pile of GLuint handles and attribute calls, and the crate was a hand-typed array of floats living in `gl_main.cpp`. Give meshes a shape — a `Mesh` class that owns its VAO/VBO/EBO — and give the crate a life as data: an OBJ file the engine loads.

## Concept

- **Ownership is the feature.** A VAO is a GPU resource with exactly one rightful owner, so `Mesh` is move-only, like a file handle: construct it from vertex data plus a layout, draw it with one call, and its destructor returns the handles (the loader grew `glDeleteVertexArrays`/`glDeleteBuffers` — by use, as always). The host's two meshes went from ~60 lines of repeated GL plumbing each to one line each, and the attribute contract (which location holds what, at which offset) now lives in exactly one place per mesh instead of being re-derived at every draw site.
- **An OBJ file is three lists and a set of references.** Positions, UVs and normals are stored once each; faces name them by index, in the forms `v`, `v/vt`, `v//vn`, `v/vt/vn`. The loader's real work is *deduplication*: a cube corner shared by three faces with different normals is three different vertices in the output, so output vertices are keyed by their (v, vt, vn) triple. Polygons become fans of triangles; a face with no normals gets a computed flat normal (the cross product of its first two edges); a missing UV becomes (0, 0). Negative indices count back from the most recent vertex, as the format specifies.
- **The loader is CPU-side on purpose.** `parseObj` touches no GL state, so the headless harness tests it like any other parser — the same separation that made the scene format and the project format testable. The shipped `assets/crate.obj` is checked too: loaded cold, it must be the 24-vertex unit cube it replaces, every coordinate exactly ±0.5.
- **Fakes are replaced, not layered.** `buildCrateMesh` is deleted, not kept as a fallback. If the OBJ fails to load, the host logs the error and skips the crates — the same honesty the missing-texture path already had.

## Code built

- `src/Mesh.hpp` / `src/Mesh.cpp` — the `Mesh` class (move-only owner; layout as (location, components) pairs) and `parseObj`/`loadObj`.
- `assets/crate.obj` — the unit cube as data: 24 vertices, 6 normals, 4 UV corners, 6 quad faces.
- `src/GLLoader.hpp/.cpp` — `glDeleteVertexArrays`, `glDeleteBuffers`.
- `src/gl_main.cpp` — terrain and crate as `Mesh` values; both draw passes call `draw()`; the hand-typed cube is gone.
- `CMakeLists.txt` — Mesh joins the GLLoader library; the harness links it for the loader checks.
- `tests/tests_main.cpp` — two OBJ checks (suite now 67).

## Verification

- Harness: **67 passed, 0 failed** — the parser check (quad fan + dedupe + computed flat normal, value-exact) and the shipped-crate check both green.
- The refactor's real proof is a *non*-difference: the L91 frame against the L90 frame from the same camera differs in **0 of 120,000 pixels**. The meshes changed ownership and the crate changed from code to file; the picture did not change at all, which is exactly what a refactor owes you. The host log confirms the path: "OBJ loaded: assets/crate.obj (24 vertices, 12 triangles)".

## Explain-it check

_(Post-handover format: why is Mesh move-only rather than copyable? In OBJ, why can one position become several output vertices? Why does the loader compute a flat normal instead of refusing a face that has none? — the answers live in the Concept section above.)_

## Next step

Lesson 92: sky & atmosphere — a gradient sky and distance fog, so the heightfield stops floating in a dark void.
