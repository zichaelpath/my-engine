# Devlog 016 — The Camera (The World Outgrows the Screen)

## Goal

Give the world a size of its own — 1600×1200 — and a `Camera` to look at it with: follow and edge-clamping in the runtime, follow/zoom/drag-pan in the editor. Generation, movement, and the camera all read the same world size; no canvas anywhere gets a vote.

## Concept

This lesson closes the question Lesson 10 opened. Zac noticed then that generation hardcoded 800×600 while `update` took the canvas size as a parameter — two competing answers to "how big is the world?" The camera forces the honest resolution:

- **The world is a fact the game owns.** `Game` gains `m_worldSize` (1600×1200). Generation scatters across all of it (the field is now 80×60 tiles; block and dot counts doubled for the larger area), movement clamps to it, and `update` no longer accepts a size from anyone. The canvas is demoted to what it always should have been: a *window*, not a boundary.
- **A camera is a centre plus a visible height.** `Camera` (GameLib, hosts own instances) holds `center` (world units) and `viewHeight` (world units). The visible width is derived from the canvas aspect ratio, so no host can ever distort the picture. Zoom is not a special feature — it is `viewHeight` changing. An `sf::View` is manufactured per frame from (centre, height, canvas size), which is why window resizes stopped needing event handling in the runtime: the view is re-derived, never maintained.
- **Follow means clamp.** A follow camera centres on its target and then clamps so the view never shows outside the world: the centre stays at least half a view from every edge (and on any axis where the view is bigger than the world, it centres on the world). Near the middle the player is centred; near a corner the camera parks and the player walks off-centre. Both behaviours are the same two lines.
- **Hosts differ, as hosts should.** The runtime has exactly one camera behaviour: follow. The editor's Game View follows too (it is a game view), but adds a follow toggle, a zoom slider, mouse-wheel zoom over the panel, and right-drag panning — the editor is allowed to look anywhere; the game is not.

## Code built

- New `src/Camera.hpp`: `Camera` struct — `viewSize`, `makeView`, `clampToWorld`, `follow`. Header-only.
- `Game`: `m_worldSize{1600, 1200}` + `worldSize()`; `playerCenter()`; `update(float dt)` (the worldSize parameter is gone); field/terrain/decor generation all read `m_worldSize` (terrain 24 blocks, decor 40 dots, field 80×60 tiles).
- Runtime: owns a `Camera`, follows the player, sets the window view from it every frame; the Lesson 9 resize handler is deleted — per-frame derivation made it dead code.
- Editor: owns a `Camera` + follow toggle + zoom slider (Engine Debug panel, with a camera readout); Game View sets its view from the camera each frame; mouse wheel zooms and right-drag pans while the panel is hovered. The Lesson 7 texture-resize machinery is untouched — texture size and view are now separate concerns, which is rather the point.

## What broke

Nothing — clean build, and all three verifications landed first try. One design detail worth recording: with the view wider than the world (zoomed far out), `clampToWorld` centres the camera and the view legitimately shows the clear colour beyond the world's edges — visible as dark bands in the editor shot. That is correct behaviour for an editor (the world has an edge; outside it is nothing), and the clamp's centre-on-world branch exists precisely for it.

## Verification

Built clean on the Linux rig. Three screenshots: (1) `lesson-16-runtime-follow.png` — the runtime at spawn: the player centred, the window showing an 800×600 window into the 1600×1200 world (the terrain is the same noise field as Lesson 14's — identical in the overlapping region, because noise is a function of world coordinates; the world simply continues past every edge now). (2) `lesson-16-runtime-corner.png` — after driving up-left for 3.5 s: the framing is pixel-identical (the camera clamped at centre (400, 300) and refused to leave the world) while the player stands in the world's top-left corner, off-centre — follow and clamp proven by the same picture. (3) `lesson-16-editor-zoom.png` — six mouse-wheel clicks over the Game View: the zoom slider reads 0.531 (0.9⁶, as coded), the camera readout shows (800, 565) — the exact centre/clamp values the math predicts at that zoom — and nearly the whole continent is visible at once, dark bands and all.

## Explain-it check

_(Post-handover format: why does the camera store a visible height instead of a width? Why did the runtime's resize handler become dead code? At a world corner, why does the player leave the centre of the screen — and why is that the camera working, not failing? — the answers live in the Concept section above.)_

## Next step

Lesson 17 — sprites & animation: entities stop being coloured rectangles. A generated sprite sheet, a texture atlas in code, and frame animation driven by game time — closing Arc 3.
