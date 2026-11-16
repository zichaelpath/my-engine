# Devlog 071 — Hot Reload

## Goal

Close the iteration loop: edit a script file, save it, and the running game picks up the new behaviour — no restart, no rebind by hand. The Game watches the files of its loaded script modules; hosts poll the watch on a throttle.

## Concept

- **The watch is a comparison, not a watcher.** No OS file-notification machinery, no threads: `reloadScriptsIfChanged()` compares each cached module's file mtime against the mtime recorded when the module was loaded (Lesson 70's bind now records it). A stat per script file, a few times a second, is all the machinery the feature needs at this engine's scale — and it behaves identically on every platform and inside the headless harness.
- **Surgical, not scorched-earth.** Only a changed file's module reloads, and only *its* instances rebind. Editing `guard.lua` never resets the state of the mover running on `mover.lua`. The blast radius of an edit is exactly the edit.
- **State resets — stated plainly.** A rebound instance gets a fresh state table: the script's `self.t`, its patrol phase, its timers all start over. That is the honest price of new code (the old state was built by logic that no longer exists), and it is the same bargain Unity and Godot strike on domain reload. What does *not* reset is the entity: position, velocity, and the world carry on — only the script's memory restarts.
- **A broken save is survivable twice over.** If the edited file fails to load, the old module keeps running, the failure is logged once, and the watch records the broken file's mtime — so it does not retry (and re-log) on every poll. The *next* save changes the mtime again and earns a fresh attempt. A reload also clears a binding's error latch (Lesson 70): fixing a crashed script and saving is exactly how it comes back to life.
- **Hosts poll; the engine decides.** Both the editor and the runtime accumulate half a second and call the watch — the editor whether or not a play-test is running, so an edit lands before you press play, not after. All policy (what counts as changed, what reloads, what rebinds) lives in GameLib where the harness can hold it.

## Code built

- `src/Game.hpp` / `src/Game.cpp` — `reloadScriptsIfChanged()`: the mtime watch, per-module reload, per-instance rebind with latch reset; `bindScripts()` records each module's file mtime at load.
- `src/main.cpp` / `src/runtime_main.cpp` — the half-second poll in both hosts' main loops.
- `tests/tests_main.cpp` — the hot-reload check (suite now 50).

## Verification

- Harness: **50 passed, 0 failed** — a mover at speed 30 runs one second (+30 exactly); an untouched watch reports 0 reloads; the file is rewritten at speed 90 with its mtime stamped forward; the watch reports exactly 1 reload; the next second moves +90; and the script's own clock, read back through the host, reads 1.0 s after the second leg — not 2.0 — proving the state table reset while the entity never teleported.
- Probe: three panels, one crate — x **200.0 → 215.0** after half a second at speed 30, then the file is rewritten at speed 240, reloaded (watch returns 1), and half a second later the crate is at **335.0**.
- Screenshot: `docs/shots/lesson-71-hot-reload.png` — the probe strip: creep, then leap, same run.

## Explain-it check

_(Post-handover format: why is the watch a poll-and-compare instead of OS file notifications? Why does only the edited script's state reset, and why does any of it reset at all? Why does a failed reload record the broken file's mtime instead of retrying on the next poll? — the answers live in the Concept section above.)_

## Next step

Lesson 72: the Lua-only NPC — Arc 5's proof scene. A patrolling NPC whose entire behaviour lives in a `.lua` file in a project's `scripts/` folder: no C++ behaviour code at all. The harness drives it through the real loop, and changing a constant in the script changes the patrol — the second pillar, demonstrated end to end.
