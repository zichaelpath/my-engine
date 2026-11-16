# Devlog 096 — glActiveTexture: Neither Platform Lets It Be Simple

## What happened

The second wall of Zac's Windows build (Nov 16, 2026), right behind Lesson 95's: `gl_main.cpp` calls `glActiveTexture` directly, and MSVC reports C3861 — identifier not found. The editor and the loader library compiled; only the GL host was left.

## The two-platform trap

`glActiveTexture` is OpenGL 1.3. Lesson 90's history: it was first added to the loader, and **Mesa's** `<GL/gl.h>` objected — Mesa declares the 1.3 functions itself, so a loader variable with the same name is a redeclaration error. It was removed from the loader the same day and called directly, which is true *on Mesa only*. The **Windows SDK's** `<GL/gl.h>` declares nothing past 1.1, so the direct call is an unknown identifier there. Each platform vetoes the other's solution.

## The fix

The function lives in the loader after all — as `glActiveTexture_`, trailing underscore, a name neither header declares and neither can collide with. One load line, four call sites renamed in `gl_main.cpp`, and the loader header's comment now tells the whole story so the next reader doesn't "simplify" it back into either trap. An audit of every direct GL call in the tree confirms the rest are all 1.1-or-older — this family is now closed by construction, not by hope.

## Verification — and an honest rig note

Verified: the full tree builds clean; the GL host runs its shadow path, which calls `glActiveTexture_` four times per frame through the freshly loaded pointer (a bad pointer would crash on the first frame, not the fourth thousand); the editor runs. The harness caveat: this morning's VM replacement on the build rig left SFML's X11 layer flaky — the harness currently aborts inside SFML's own display-open after 36 passing checks and zero failures, in code the diff provably does not touch (`MyTests` contains no loader symbols at all; its sources are unchanged). It read 69/69 earlier today on the pre-replacement rig. The Windows confirmation, as with Lesson 95, is Zac's rebuild — for a Windows-only compile error, his MSVC is the only honest oracle anyway.

## The lesson inside the lesson

"Declared by the header" is a property of a *platform*, not of a function. The loader's rule is now complete: if a function is newer than 1.1, it belongs to the loader — and if a platform header disagrees, the pointer changes its name, not its home.
