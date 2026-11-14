# Devlog 001 — The Loop

## Goal

A window, a game loop, and a frame clock: open an 800×600 SFML window titled "MyEngine - Lesson 1", run the loop until it closes, and print the frame rate once per second.

## Concept

An engine is a loop that repeats three steps forever: **Input** (did anything happen?), **Update** (change the world), **Draw** (show it). The clock measures **dt** — delta time, the seconds one frame took — because everything that moves will move by *speed × dt*, not by a fixed amount per frame. `clock.restart()` returns the elapsed time and resets the clock to zero in the same call, so every frame starts timing the next one immediately.

## Code built

- CMake + FetchContent skeleton that downloads SFML 3.0.2 and builds `MyEngine` from `src/main.cpp` (C++20).
- The main loop: window, `sf::Clock`, an `int frames` counter and a `float fpsTimer`.
- Event handling: window close and Esc both exit.
- FPS printout: frames counted, dt accumulated, and once a second it prints FPS and average frame time (`fpsTimer / frames * 1000` ms).

## What broke

—

## Verification

Built and ran on my laptop. Uncapped, the loop ran at **FPS 7482, 8457, 8607, 8352, 8540, 8424** across the printed seconds, with frame times of about **0.116–0.134 ms**. The X button and Esc both close the window.

## Explain-it check

In my own steps:

1. Create the SFML window, 800×600.
2. Create a clock for ticks/FPS, plus an `int frames` and a `float fpsTimer`.
3. The loop starts and dt comes from `clock.restart()`, which stores the elapsed time and resets the counter in one move.
4. X or Esc exits the loop.
5. `frames` increments and `fpsTimer` accumulates dt, printing FPS and per-frame ms (`fpsTimer/frames*1000`) each second.
6. Clear the window with a colour, then display.

## Next step

Lesson 2 — draw a shape and move it with the keyboard: speed × dt, diagonal normalisation, and clamping at the walls.
