# Devlog 045 — The Probes Grow Up: A Test Harness

## Goal

Arc 1's second lesson, straight from the Phase 2 plan: turn Phase 1's throwaway verification probes — the Lesson 38 grid checksum, the Lesson 44 flip measurement — into a permanent citizen: a `MyTests` executable that checks the engine's load-bearing promises in one command and reports an exit code a machine can read.

## Concept

- **Test the promises, not the implementation.** The ten checks are the engine's contracts, stated in Phase 1's own words: same seed, same world (cell-by-cell, two fresh `Game`s); the world's fingerprint never changes silently (an FNV-1a checksum over the grid, baseline **936912399**, recorded from the shipped 1.0 code — Lesson 38's probe proved the property, this makes it a gate); a different seed is a different world; a scene survives a save/load round-trip with every entity intact; a project file loads its fields, refuses a future format *without touching the caller's struct*, and treats a missing file as a clean false; config values land, defaults fill gaps, and a malformed line is skipped, never fatal; and the Lesson 44 regression — a left-facing sprite renders inside its entity box — checked by rendering, forever.
- **SKIP is a first-class result.** A check that cannot run in an environment must say so, never fake a PASS and never cry FAIL over missing hardware. The render check skips when no render context exists; the whole suite skips cleanly on a display-less Linux box (below). A red X should always mean *the engine* broke.
- **A harness that crashes must not take its results with it.** Output is unbuffered from the first line. The first run of this harness proved why: it aborted (below) and a buffered version would have reported nothing at all.
- **The engine's log stays on.** `Game` narrates its construction into the harness output — generation, spawns, scene loads, the project refusal error. That interleaving is a feature: when a check fails in CI, the engine's own account of what it was doing is already in the log.

## Code built

- `tests/tests_main.cpp` — the harness: ten checks, `[PASS]/[FAIL]/[SKIP]` lines, a `RESULT` summary, exit code = 1 on any failure.
- `CMakeLists.txt` — the `MyTests` target (links GameLib only); and a structural fix the harness immediately earned: **GameLib now exports its `src/` include directory PUBLIC**. Until today every consumer found the headers by living in `src/` itself; the first outsider (tests/) exposed that the library never actually published its own headers.
- One API correction on the way: SFML 3's `sf::RenderTexture` has no `create()` — it is constructed with its size, and failure reads back as size {0,0}. Verified against the pinned headers, per the standing rule.

## What broke

- **The first run aborted with no results.** Two discoveries inside it. (1) On this display-less rig the audio stack fails loudly (miniaudio/ALSA noise) but harmlessly — expected, and the hosts live with it too. (2) The real one: **a `Game` cannot even be constructed without a display.** The sprite sheet is an `sf::Texture` — a GL resource — so construction itself needs a context, and SFML aborts when X11 is unavailable. Even the *non-render* checks need a display. The harness now detects a missing `DISPLAY` up front (Linux) and skips the suite honestly; under `xvfb-run` everything executes. This is also the answer for CI (next lesson): the harness runs under a virtual display, by design, not by accident.
- The build rig's missing-header saga from Lesson 44 got its permanent answer in passing: the full build is green again on this machine, harness included.

## Verification

- With a display: **10 passed, 0 failed, 0 skipped**, exit 0 — run twice, checksum stable at 936912399 both times.
- Without a display: one honest SKIP line for the suite, exit 0, no abort, no lost output.
- Full project build after the CMake changes: all targets green (the GameLib include change disturbed nothing).
- Screenshot: `docs/shots/lesson-45-tests.png` — the harness's own output from the verified run, rendered verbatim.

## Explain-it check

_(Post-handover format: why is a checksum baseline a different kind of check from comparing two fresh worlds? Why does the suite skip rather than fail on a display-less machine — and what makes that safe rather than lazy? Why did adding a test target change GameLib's CMake? — the answers live in the Concept section above.)_

## Next step

Lesson 46: CI — GitHub Actions building Linux and Windows on every push and running this harness under `xvfb-run`, so the checks stop depending on anyone remembering to run them.
