# Devlog 070 — Scripts in the Loop

## Goal

Make scripts *run*. The Game owns a `ScriptHost`, binds an instance for every scripted entity when a scene loads or a run starts, and calls each instance's `onUpdate` inside `update()` — with a per-script error latch so a broken script logs once instead of sixty times a second. Entity script paths resolve against the project's root, set by both hosts.

## Concept

- **The Game owns the host.** `ScriptHost` is a member, and the entity API (Lesson 67) is installed once, in the constructor, against `*this`. There is no global VM and no singleton: two Games in one process (the harness makes dozens) each carry their own Lua state, and a script can only ever touch the Game that bound it.
- **Binding is an event, not a state.** `bindScripts()` runs exactly when the entity list is replaced — the constructor's first world, `newRun`, `loadScene` — and on demand. Modules are cached by resolved path (ten guards on one script share one module, ten state tables); instances are the per-entity state tables from Lesson 67. Editor undo/redo deliberately does *not* rebind: instances resolve their entity by name on every call, so a binding survives an undo that restores a same-named entity list.
- **Placement inside the frame matters.** The script step sits after platforms move and before movement integration. A velocity a script sets is integrated *this* frame — no one-frame lag between a script's decision and the world's answer — and `getPosition` reads last frame's settled position, the stable ground every engine's update order promises. Scripts also run while the game sits on its title or won screens: the world stays alive there (Lesson 52's stance), and scripts are world behaviour, not input handling.
- **The error latch.** When an instance's `onUpdate` fails, the host logs the failure (its standing job) and the binding latches off until the next bind. A broken script costs exactly one log line and then silence — it never spams, and it never keeps half-running on a state its own error may have left corrupt. Fixing the file and rebinding (next lesson: automatically) gives it a fresh chance.
- **Scripts have an address.** An entity's script path is project-relative — Arc 4's rule for every resource — and `setScriptRoot` says where relative paths resolve. Both hosts set it from the loaded project: the runtime at startup, the editor at startup *and* in the open-project handler, where the root is set **before** `newRun` because the bind happens inside it. An empty root means the working directory, which is what tests and tools want.

## Code built

- `src/Game.hpp` / `src/Game.cpp` — the owned `ScriptHost`, the module cache, the binding list with its latch, `bindScripts()` called from the constructor, `newRun`, and `loadScene`; the script step inside `update()`; `setScriptRoot` / `scriptHost()` / `scriptBindingCount()` accessors.
- `src/main.cpp` — the editor sets the script root at startup and on project adoption (before `newRun`).
- `src/runtime_main.cpp` — the runtime sets the script root at startup, before the start scene loads.
- `tests/tests_main.cpp` — the scripts-in-the-loop check (suite now 49).

## Verification

- Harness: **49 passed, 0 failed** — one check, four clauses: two scripted crates bind; sixty real `update()` calls move the mover exactly +30; the erroring script's own counter reads back exactly **1** after sixty frames (the latch, proven from inside Lua); and a saved scene loaded into a fresh Game binds both scripts with no explicit bind in between — the file goes live on load.
- Probe: a crate whose only mover is `scripts/mover.lua`, rendered before and after sixty updates — the engine's own getter reads **x 200.0 → 230.0**, bindings = 1.
- Screenshot: `docs/shots/lesson-70-scripts-loop.png` — the probe strip; the crate (placeholder checkerboard — the probe ran from a scratch folder, Lesson 44's old lesson) sits 30 units further right in the after panel, put there by Lua.
- Rig note: the build VM was replaced again during this lesson (system headers and linker symlinks wiped, `~` intact); the standing recovery recipe — versioned-library symlinks plus old-mtime placeholder headers — restored the build before any engine code changed.

## Explain-it check

_(Post-handover format: why does the script step sit before movement integration rather than after it? Why latch a failed script off instead of retrying it next frame? Why must the editor set the script root before calling `newRun` when a project opens? — the answers live in the Concept section above.)_

## Next step

Lesson 71: hot reload — watch script files for changes; when one changes, reload its module, rebind its instances (state resets, logged), and the edited behaviour takes over without restarting the game.
