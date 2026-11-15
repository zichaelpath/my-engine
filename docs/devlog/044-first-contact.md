# Devlog 044 — First Contact: The Left-Facing Ghost

## Goal

Phase 2 opens the way its roadmap promised: with Zac's own playtest. He pulled 1.0.0, built it on Windows, played it, and came back with a screenshot and a verdict — *"look at the hitbox of the player as outlined in this photo. something somewhere is wrong with them."* This lesson is that finding chased to ground. A second finding from the same session (a Windows build break) rides along and ships first.

## Concept

- **The hitbox was innocent; the picture was lying.** Every system that touches the player's body — collision (Game.cpp:536), gold collection (the feet centre, :438), world bounds (:397) — reads `position + hitboxOffset`, and the editor's overlay draws exactly that box. In Zac's screenshot the red feet box sits a full body-width *left* of the visible character. One of the two is displaced, and the code says the box can't be: it's the sprite.
- **The flip math, and where it double-counts.** To mirror a sprite in place you do two things: shift its position by `+size.x` and negate its x scale, so the texture's left edge lands on the box's right edge and unfolds leftward across the box. SFML's `origin` is a *second* shift applied to the same vertices before scaling. The Lesson 17 flip branch set origin `(FRAME, 0)` **on top of** the position shift — so a flipped sprite occupied `[pos + 50, pos + 100]` while its hitbox lived at `[pos, pos + 50]`. Facing right (no flip), the two coincide perfectly, which is why the crate's red box in the same screenshot sits pixel-perfect on its crate while the player's floats beside him.
- **Why it survived 27 lessons.** `flipX` is only true while moving left, and stays true when you stop (Game.cpp:332 — facing persists). Every verification screenshot since Lesson 17 caught the player facing front or right; the collision numbers were always verified directly (Lesson 19's rest at exactly (240,153)) — and collision was never wrong. The bug needed a *player* to notice that the visible man and his body had parted company. That is literally what Arc 1 exists for.
- **Diagnosis by measurement, not eyeballs.** Zac's screenshot was pixel-measured: red-outline clusters extracted programmatically (two boxes: a 40×40 around the checkerboard crate, a 28×22 at the player's feet height but ~49 world units left of the sprite, whose exact bounds came from the painted palette in SpriteSheet.cpp). Then a headless probe — a throwaway program that renders `Game` to a texture with `flipX` forced on and hitboxes shown, and prints the hood colour's pixel span — reproduced it against the engine's own rendering. That probe is the seed of Arc 1's test harness.
- **Finding #2: a clean checkout wasn't.** Zac's MSVC build died in GLLoader: `glext.h` line 37 includes `<KHR/khrplatform.h>`, which Linux had been quietly supplying from system headers and Windows does not have (C1083). The header now lives at `vendor/KHR/khrplatform.h`, straight from the Khronos registry, beside the `glext.h` that needs it — shipped as its own commit (`e0e4017`) so the unblock reached him immediately.
- **Not a bug: the magenta crate.** The checkerboard square in his screenshot is Lesson 20's placeholder doing its job — the editor was launched from the Build folder, so `assets/crate.tga` wasn't under the working directory. The rule already lives in `docs/WINDOWS_BUILD.md` (run from the repo root); now it has a photo of why.

## Code changed

- `src/Game.cpp` — the flip branch: origin `(FRAME, 0)` → `(0, 0)`, with a comment recording why, so nobody "fixes" it back.
- `vendor/KHR/khrplatform.h` — vendored Khronos header (commit `e0e4017`).
- The probe lived in /tmp and died there; its grown-up version is Arc 1's test-harness lesson.

## What broke

My build rig, not the engine: the VM was replaced mid-lesson and took the system `-dev` symlinks and several headers with it (libogg/libFLAC/libXfixes/libGLX symlinks; FLAC/vorbis/Xfixes headers). The rig was restored with versioned-library symlinks and old-mtime placeholder headers so the cached SFML objects stay valid; the full build is green again. Recorded here so a future strange link error on this machine gets diagnosed in minutes, not an hour.

## Verification

- **Probe, before the fix:** hood pixels span x **438..462** — the sprite entirely right of its entity box (375..425), matching the ~+49 units measured in Zac's screenshot.
- **Probe, after the fix:** hood pixels span x **388..411** — inside the entity box and centred over the feet box (386..414). Displacement: one full body-width → zero.
- Full build after both changes: all three hosts green, GLLoader compiling against the vendored `khrplatform.h` on Linux as well.
- Screenshots: `docs/shots/lesson-44-before.png` / `lesson-44-after.png` — probe renders, player facing left, hitboxes on; the red feet box moves from beside the character to under him.

## Explain-it check

_(Post-handover format: why did the overlay accuse the hitbox when the sprite was the liar? Why does a negative scale need the +size.x position shift but break with an origin on top? Why did this only ever show when facing left? — the answers live in the Concept section above.)_

## Next step

Arc 1 continues on Zac's machine: re-pull, rebuild on Windows (the khrplatform fix), confirm the left-facing body is honest, and give the audio its first real-hardware listen — the one Phase 1 verification this rig could never do. Then the planned Arc 1 machinery: the test harness, CI, and 1.0.1 to ship both fixes.
