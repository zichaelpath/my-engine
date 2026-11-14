# MyEngine

My own game engine, built from scratch, one lesson at a time — and documented as I go in the devlog (`docs/devlog/`). The end goal is to ship it as a portfolio piece: an engine library with an editor and a runtime, the way Godot or Unity are shaped, at my own scale.

**Stack:** C++20, CMake, SFML 3 (window/input/audio for the 2D phase). Dear ImGui joins in Lesson 3 for internal editor windows; raw modern OpenGL comes later for the 3D rendering core. Procedural generation is planned as a foundation of the engine, not an add-on.

## Build (Windows, VS Code)

```
cmake -B Build
cmake --build Build
```

Run it:

```
Build/Debug/MyEngine.exe
```

Right now (Lessons 1–2) it's a window, a loop with an FPS counter printed to the console once a second, and a green square you move with WASD or the arrow keys. Esc closes it.
