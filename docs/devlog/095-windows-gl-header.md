# Devlog 095 — The Playtest Gate Earns Its Keep: Windows vs <GL/gl.h>

## What happened

Zac's first Windows build of the 1.8.0 tree (Nov 16, 2026) built `MyRuntime.exe` cleanly — both games playable — and then died compiling `GLProgram.cpp`: a hundred errors inside the Windows SDK's own `GL\gl.h`, all variations on `WINGDIAPI` and `APIENTRY` being undefined. The editor and the GL host were blocked; the runtime, which links no GL, was not.

## The cause

The Windows SDK's `<GL/gl.h>` is not self-contained: it spends macros that `<windows.h>` defines, so Windows must be included first. Mesa's `<GL/gl.h>` on Linux *is* self-contained, so the Linux rig — every build this project has ever verified on — structurally cannot see this failure.

`GLLoader.hpp` has carried the correct preamble since the Phase 1 Windows fixes. The two headers Lesson 93 introduced — `GLProgram.hpp` and `Viewport3D.hpp` — included `<GL/gl.h>` bare. `gl_main.cpp` survived by accident of ordering (SFML's headers reach Windows first); `GLProgram.cpp` includes its own header first, so it was the first to fall.

## The fix

Both headers now carry the same platform preamble as the loader — `WIN32_LEAN_AND_MEAN`, plus `NOMINMAX`, which matters most for `Viewport3D.hpp`: it is included by the editor's `main.cpp`, whose `std::clamp` calls the `min`/`max` macros would otherwise silently corrupt. An audit of the tree found no other bare `<GL/gl.h>` inclusions; `GLLoader.cpp`'s `<GL/glx.h>` sits in its non-Windows branch, where it belongs.

## Verification

Linux is unaffected by construction (the preamble is `_WIN32`-gated) and confirmed so: full rebuild, harness **69 passed, 0 failed**, the GL host compiles all eight shaders and runs, the editor runs. The Windows confirmation is Zac's rebuild — the gate working as designed: his machine is the only Windows compiler this project has, and this devlog exists because it spoke up.

## The lesson inside the lesson

Every header that includes a platform header owns the platform's inclusion rules — "it compiled on the rig" proves exactly one platform. New headers that touch GL now start from the loader's preamble by default; when Arc 10's showcase pass writes the portability notes, this entry is exhibit A.
