# Devlog 009 — The Runtime (One Game, Two Hosts)

## Goal

Run the same `Game` in a second program: a bare SFML window with no ImGui in it at all. To share the game properly, `Game.cpp` becomes a **library** (`GameLib`) that both executables link — the first true Editor/Runtime split.

## Concept

An executable is a `main()` plus everything it links. A **library** is compiled code with no `main()` of its own, built once and linked into any number of programs. Until now `Game.cpp` was listed inside the one executable; listing it inside two executables would compile it twice and keep the game's code in two build definitions. As a library, the game is a single artifact — one compilation, one definition — that hosts share.

CMake linkage carries visibility:

- `target_link_libraries(GameLib PUBLIC SFML::...)` — **PUBLIC** means "I need this, and so does anyone who uses me." `Game.hpp` includes SFML headers, so every consumer of the library needs SFML on its include path; PUBLIC propagates it automatically.
- The executables link the library **PRIVATE** — they use it, but nothing links *them*.

The Runtime host itself is deliberately tiny, and its differences from the editor are the lesson:

- **No pause.** It calls `game.update(dt, ...)` every frame, unconditionally. Pause never belonged to the game — it was editor policy, a host deciding whether to call update. The game doesn't miss it.
- **The window is the canvas.** `game.render(window)` — the same `render(sf::RenderTarget&)` the editor points at a texture. And the job Lesson 7's panel machinery did in the editor (keep the view fitted to the canvas) is done here by handling the window's `Resized` event and resetting the view.
- **Zero ImGui** — no includes, no linked library — yet the game compiles and runs. Lesson 8's separation test, made flesh.

One trap, named in advance: `src/Game.cpp` must be **removed** from `add_executable(MyEngine ...)` when it moves into the library, or the game gets defined twice (once in the executable, once in the library) and the linker fails with duplicate symbols.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- `CMakeLists.txt`: new `add_library(GameLib src/Game.cpp)` with PUBLIC SFML links; `MyEngine` now built from `src/main.cpp` only and links `GameLib` + ImGui-SFML; new `add_executable(MyRuntime src/runtime_main.cpp)` linking only `GameLib`.
- New `src/runtime_main.cpp`: plain window host — event loop (close/Esc, Resized → reset view), clock, unconditional `game.update(dt, window size)`, `game.render(window)`, `display()`. No ImGui anywhere.
- Editor unchanged in behaviour; it just receives the game from the library now.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — both executables build; MyRuntime.exe opens a bare window, game playing immediately, resize keeps the world unstretched, Esc closes; editor behaviour unchanged)_

## Explain-it check

A library can be reused by every executable instead of repeating massive build lists as time goes on. The game's .cpp files are compiled into machine code once; each program just links the finished library rather than compiling those files again and again.
The runtime has no pause because we were using the pause in the game engine itself to control the flow of the building and debugging. When it comes time to adding a pause menu in any of the games made with this engine, THEN we can let the developers create their own "game pause".
Lesson 7's job looks as if we dynamically set the game's world size based on the resizing of the window based on the events of the window being resized. even though the resize happens at the game window event level, we still have to pass that size into the game's update function. otherwise you'll get a distorted mess when the game is suddenly maximized.

## Next step

Lesson 10 — to be chosen: a second entity and simple collision (the game grows), or the master-seed procgen foundation the engine was always meant to have.
