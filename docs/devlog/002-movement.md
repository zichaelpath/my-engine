# Devlog 002 — Movement

## Goal

Draw a shape and move it with WASD and the arrow keys at a steady speed using dt, keeping it clamped inside the window.

## Concept

- Screen coordinates start at the **top-left** corner, and y goes **down**. A shape's position is its top-left corner.
- Movement input is **polled**, not event-based: `isKeyPressed` asks "is this key held *right now*?" every frame, which is what continuous movement needs. The event loop is for discrete things — a key *press*, a resize, a close.
- A diagonal direction (1, 1) is longer than (1, 0) or (0, 1): by Pythagoras its length is √2 ≈ 1.41. Dividing the direction by its length (**normalising**) brings the distance back to 1, so diagonals aren't faster.
- **Clamping** keeps the position between 0 and (window size − shape size), so the player can never leave the screen.
- Movement is **speed per second** via dt, not speed per frame. Capping the frame rate changes dt; it doesn't change how far the player travels in a second.

## Code built

- A 50×50 green `sf::RectangleShape` starting centred at (375, 275).
- WASD + arrow-key polling into a direction vector each frame.
- Diagonal normalisation: `len = sqrt(x² + y²)`, divide both components by `len` when `len > 0`.
- Position update: `pos = position + dir * speed * dt`, with `speed` = 200 pixels per second.
- Position clamping: x to 0–750, y to 0–550.
- The `setFramerateLimit(60)` experiment from Lesson 1's uncapped loop.

## What broke

—

## Verification

Built and ran on my laptop: the shape moves smoothly in all eight directions, diagonals are the same speed as straight movement, and it stops dead at all four walls instead of sliding off-screen. With the frame rate capped at 60 the movement covers the same distance per second as uncapped — dt grows to 1/60 (vs ~1/8000 uncapped), so each frame's step is bigger but there are fewer frames.

## Explain-it check

- **Why polling for movement:** movement is continuous — you hold the key and the shape keeps going — so the code asks every frame whether the key is held right now. The event loop is for discrete engine controls, like resizing the window or closing it: things that happen once, not things that are held.
- **The len fix:** the (1,1) direction is longer than (1,0) or (0,1); by Pythagoras its length is √2, and dividing by len brings the direction distance back to 1 — that's normalising, not clamping.
- **Walls:** a clamp on position, so the player stays on screen.
- **Frame-rate independence:** movement is speed per second via dt, not speed per frame. The 60 FPS cap proves it — it doesn't create it.

## Next step

Lesson 3 — the first internal window: a Dear ImGui debug panel inside the engine window.
