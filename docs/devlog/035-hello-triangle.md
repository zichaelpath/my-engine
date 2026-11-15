# Devlog 035 — Hello, Triangle (Arc 8 Opens)

## Goal

Open the 3D arc: a third host, `MyGL`, that opens a raw OpenGL 3.3 core context in an SFML window and draws through the engine's own shader pipeline — nothing rendered by SFML.

## Concept

- **The loader is not magic, so we wrote it.** A core context only guarantees OpenGL 1.1 entry points at link time; shaders, buffers, and vertex arrays must be requested from the driver *by name* at runtime. GLAD generates that code — this engine writes it: `GLLoader` holds one pointer per post-1.1 function the engine uses (19 today), filled by `glXGetProcAddressARB` on Linux / `wglGetProcAddress` on Windows once the context is current, with every miss logged by name. It will grow by exactly one line per new function an arc lesson needs — a census of the engine's GL surface. The PFN types come from the official Khronos `glext.h`, vendored into the repo (MIT-licensed) so both platforms build identically.
- **SFML is demoted, deliberately.** `MyGL` uses `sf::Window` — window, context, events — never `sf::RenderWindow`, so no SFML renderer state exists to fight the GL state machine. The 2D hosts are untouched and keep their renderer; the two paths share `GameLib`'s non-rendering parts and nothing else.
- **The pipeline, end to end, once:** context (3.3 core requested) → load functions → shader *source* → compile each stage (checking the info log, the only error channel GLSL has) → link a program → describe the vertex layout (interleaved position+colour in one VBO, captured by a VAO) → draw. The triangle's colour is three vertex colours interpolated by the GPU across the face — that smooth gradient is the receipt that every stage above actually worked.
- **Version numbers, honestly:** we requested 3.3 core; Mesa answered with a 4.5 core-profile context. That's correct GL behaviour — a core context at or above the request — and the program still uses only 3.3 features, which is what keeps it portable to Zac's machine.

## Code built

- `src/GLLoader.hpp/.cpp` — the loader (19 functions), linked as its own library; platform GL lib only.
- `vendor/GL/glext.h` — the Khronos header, vendored.
- `src/gl_main.cpp` — the `MyGL` host: context settings, shader compile/link with info-log reporting, VAO/VBO setup, the gradient triangle.
- CMake: `GLLoader` library + `MyGL` target; `MyEngine`/`MyRuntime` unchanged and still building.

## What broke

Nothing. The build was clean on the first attempt — the API details (SFML 3's `Window(mode, title, State, settings)` constructor and `ContextSettings::Attribute::Core`) were checked against the pinned SFML headers before writing the code, per the standing rule. The run log's "Setting vertical sync not supported" is an SFML notice under Xvfb, unrelated.

## Verification

Run under the rig: the log shows the full chain — "OpenGL functions loaded", **"OpenGL version: 4.5 (Core Profile) Mesa 25.2.8"**, "Vertex shader compiled", "Fragment shader compiled", "Shader program linked" — and the screenshot shows the gradient triangle, red/green/blue at the corners, interpolated across the face, on the dark clear colour. A black window would have meant a dead pipeline; a flat-coloured triangle would have meant the attribute layout was wrong. The gradient is the test, and it passes by eye.

## Explain-it check

_(Post-handover format: why can't a 3.3 program just link glCreateShader the way it links glClear? What does the VAO remember that the VBO doesn't? Why is the fragment colour smooth when we only ever named three colours? — the answers live in the Concept section above.)_

## Next step

Lesson 36 — the third dimension for real: an indexed cube, the depth test, and the MVP matrix chain (model rotation, view, perspective projection) — the triangle grows up into a spinning cube, and the math library gets its first 4×4 matrices.
