# Devlog 051 — Camera Feel: Trauma, Hit-Stop, and a Camera That Glides

## Goal

Arc 2's third lesson gives the camera a nervous system. Three pieces, all in GameLib or the shared `Camera`, all wired into both hosts: **trauma-based shake** (events add shake budget; the camera spends it), **hit-stop** (the world freezes for a beat at the win), and a **follow camera that glides with lookahead** instead of snapping to the player.

## Concept

- **Trauma, not shake amount.** Events never set a shake directly; they *add trauma* (a wall bump +0.15, the winning dot +0.35), and the visible offset scales with **trauma²**. The square is the whole trick: at trauma 0.2 the world barely trembles, at 1.0 it rattles at full amplitude, and two small events close together still don't equal one big one. Trauma decays linearly (1.4/s) and the offset itself is a pure function of game time — two incommensurate sines — so it is smooth, irregular, and exactly reproducible in a test. No RNG in the camera, ever.
- **Hit-stop must not eat time.** The first version of the freeze discarded the remainder of any frame in which the stop expired — the harness caught the run clock losing that sliver, and the fix carries the leftover dt into the world update. A 120 ms freeze should cost the player exactly 120 ms, not 120 ms plus however the frames happened to fall.
- **Lookahead is velocity with a leash.** `Camera::follow` now leads the target by 0.18 s of its *actual post-collision* velocity (Game tracks it per frame), the lead capped at 36 units, and approaches exponentially — `1 − exp(−8·dt)` of the remaining distance per frame — so the glide feels identical at 30 fps and 240 fps. The camera shows you where you're *going*, which is most of what "smooth" ever meant.
- **The game owns feel; hosts own views.** Game computes `shakeOffset()`; each host adds it to its view centre (`view.move(...)` after `makeView`). Editor and Runtime rattle identically because neither implements shake — they borrow it.

## Code built

- `src/Game.*` — trauma (`addTrauma`/`trauma`/`shakeOffset`), hit-stop (`hitStop`/`isHitStopped`, leftover-dt carry), per-frame player velocity, triggers: bump → +0.15 trauma; final gold → 0.12 s stop + 0.35 trauma.
- `src/Camera.hpp` — `follow` takes velocity + dt: capped lookahead, exponential glide.
- `src/main.cpp`, `src/runtime_main.cpp` — new follow arguments; the shake offset applied to the world view in both hosts.
- `tests/tests_main.cpp` — three new checks (suite now 21).

## What broke

The hit-stop's first draft ate frame remainders (above) — a real bug the harness was written just in time to catch, and the reason the check asserts the clock *resumes*, not merely that it froze. Also the build rig's Xvfb died between commands again (the second VM replacement of the evening had already been rebuilt; the display service itself is simply flaky) — restarted from the standing recipe, suite re-verified green.

## Verification

- Harness: **21 passed, 0 failed.** Shake: calm at rest, peaks > 4 units under full trauma, never beyond the 14-unit bound, decays to exactly zero offset. Hit-stop: run clock frozen mid-stop, resumed after, no time eaten. Camera: one frame glides part-way (never snaps), 240 frames settle within 1.5 units of the capped lookahead point (436 = spawn 400 + 36).
- Probe: at a sampled instant under full trauma, `shakeOffset()` read **(−8.4, +7.5)**; the rendered frame through the hosts' exact recipe is displaced by precisely that.
- Screenshot: `docs/shots/lesson-51-shake.png` — calm (left) vs rattling (right), same world, same instant, the measured offset between them.

## Explain-it check

_(Post-handover format: why does the offset scale with trauma² instead of trauma? Why is the shake a function of time rather than random numbers? Why did the first hit-stop lose time, and what does the fix do with a frame's leftover dt? — the answers live in the Concept section above.)_

## Next step

Lesson 52: Gold Rush, juiced — the whole feel layer aimed at the shipped game at once (anticipation on the pop, landing dust when the player stops short, a win sequence that stages its effects), verified by playing a full run headlessly start to finish.
