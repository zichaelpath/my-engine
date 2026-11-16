# Devlog 066 — Lua Lives Here

## Goal

Arc 5 opens by embedding the language: **Lua 5.4.7**, fetched from the official lua.org tarball with its SHA-256 pinned (computed from the actual download, not remembered), built from its C sources as a static library, and wrapped in a `ScriptHost` — one Lua state with a small C++ face: run a string, run a file, evaluate to a number. Nothing calls scripts yet; this lesson makes Lua *present and safe*.

## Concept

- **Hand-bound means owned.** Lua ships no CMakeLists, so the build is written here: every `.c` in its `src/` except the two standalone programs (`lua.c`, `luac.c`) becomes the `lua` library, linked into GameLib so the editor, the runtime, and the harness all share it. This is the GL-loader philosophy applied to a language (and the choice Zac made on Nov 15): the embedding is small enough to read end to end, and nothing about it is magic. It also required the project to admit it speaks C now — `LANGUAGES C CXX`.
- **Errors are values.** Every entry point loads with `luaL_loadbufferx`/`loadfilex` and runs under `lua_pcall`; a syntax error, a runtime `error()`, or a missing file logs its message through the engine's Log and returns false. The harness proves the host survives all three *and keeps working afterwards* — the property Lesson 70 will rely on when scripts run inside the frame loop: a game's typo must cost a log line, never the process.
- **The trust model is stated, not implied.** The host opens Lua's standard libraries in full. Scripts are a game's own code — written by its author, shipped in its `scripts/` folder, trusted exactly the way the game's own C++ would be. Sandboxing other people's code is a different feature for a different day, and pretending this is one would be a lie in the documentation.
- **The header stays clean.** `ScriptHost.hpp` forward-declares `lua_State` and includes no Lua headers; only the `.cpp` files that bind functions (next lesson) will include them. Includers of the host don't inherit a C API they didn't ask for.

## Code built

- `CMakeLists.txt` — the Lua FetchContent block (pinned hash), the `lua` static library, `LANGUAGES C CXX`, GameLib links it and gains `ScriptHost.cpp`.
- `src/ScriptHost.hpp/.cpp` — the host: construction opens the standard libraries; `runString`, `runFile`, `evalNumber`, all error-as-value.
- `tests/tests_main.cpp` — the host check (suite now 45).

## Verification

- Harness: **45 passed, 0 failed** — `return 6 * 7` evaluates to 42; a global set by one chunk reads back in another (state persists); a syntax error and a runtime `error('boom')` both return false with logged messages and the host answers the next eval; a file runs and its global reads back; a missing file is a false.
- The build reconfigured cleanly with the new dependency (FetchContent pulled the tarball by its pinned hash).
- Screenshot: `docs/shots/lesson-66-lua-host.png` — the host's actual log lines from the harness run.

## Explain-it check

_(Post-handover format: why is the tarball's hash pinned in CMake instead of fetching "latest 5.4"? Why does every script run go through `lua_pcall` instead of a plain call? Why open the full standard libraries instead of a sandboxed subset? — the answers live in the Concept section above.)_

## Next step

Lesson 67: the bridge — the hand-bound engine API v1 that scripts will actually call: entity position get/set and movement, logging, and game time, with per-entity script state.
