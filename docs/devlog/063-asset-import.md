# Devlog 063 — Import: Assets Move Into the Place

## Goal

The file browser's file mode gains its consumer. **Import asset…** in the Project panel opens the browser; the chosen file — from anywhere on disk — is copied into the project's `assets/` by `importAsset`, and the panel refreshes and selects what arrived. A project stops depending on files scattered across the machine: what the game uses, the project *contains*.

## Concept

- **Import copies; it never moves, never overwrites.** The source file stays exactly where it was — the Downloads folder keeps its file; the project gains its own. And a name that is already taken in `assets/` earns the next numeric suffix (`hero.png` → `hero-2.png` → `hero-3.png`) rather than replacing anything: an import that silently replaced an asset the game already uses would be a data-loss feature wearing a convenience costume. Existence is checked explicitly before copying, and the copy itself runs without the overwrite option — two locks on the same door.
- **The result is a project-relative path.** `importAsset` returns where the file landed *inside the project* (`assets/hero-2.png`) or an empty string on failure — the same currency the panel's selection and, later, scene files and scripts trade in. Callers never reconstruct paths; the module that owns the layout reports them.
- **Failure is a value, not a crash.** A missing source, a rootless project, a failed copy: all return "" with a logged reason, and the panel simply doesn't change. The harness asserts all of it, including that the imported bytes arrive intact and the source survives.

## Code built

- `src/Project.hpp/.cpp` — `importAsset` (creates `assets/` when needed, collision-suffix loop, logged outcomes).
- `src/main.cpp` — the Import button (browser purpose 2, starting in the working directory) and the `dialogFinished` consumer: import, refresh, select.
- `tests/tests_main.cpp` — the import check (suite now 42).

## Verification

- Harness: **42 passed, 0 failed** — importing an outside `hero.png` into a project that already owns one lands at `assets/hero-2.png`, a second import at `assets/hero-3.png`, contents byte-identical, source untouched, a missing source returning "".
- Probe: the same behaviour against a real on-disk project (screenshot source).
- The editor runs with the new wiring (live smoke test).
- Screenshot: `docs/shots/lesson-63-import.png` — the probe's actual import outcomes.

## Explain-it check

_(Post-handover format: why does import copy rather than move or link? Why does a name collision earn a suffix instead of an overwrite prompt or a refusal? Why does the function return a project-relative path instead of a success flag? — the answers live in the Concept section above.)_

## Next step

Lesson 64: scenes as files — the scene gains a current-file path inside `scenes/`, with Save / Save As / Load through the browser, and double-clicking a scene in the Project panel loads it.
