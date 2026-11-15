# Devlog 017 — Sprites & Animation (The Rectangles Retire)

## Goal

Give entities pictures. Build a sprite sheet in code (no image files), cut sprites from it by rectangle, and animate the player's walk cycle from game time — with facing flips. Arc 3 closes.

## Concept

Three ideas carry the whole lesson:

- **A texture is an atlas, a sprite is a window into it.** One 160×32 texture holds everything: four 32×32 walk frames of the player and one crate cell. A sprite is just (texture, rectangle, position, scale) — the *frame* of an animation is nothing but the rectangle's x offset changing. There is no animation file format, no timeline object: time passes, an index advances, the window slides.
- **The sheet is generated, not loaded — for now.** `buildSpriteSheetImage()` paints the characters pixel by pixel into an `sf::Image` at startup (a hooded traveller: hood, face, cloak, belt; legs apart / legs together with a 1px bob — the oldest walk-cycle trick there is; the crate gets planks, a diagonal brace, and nails). This keeps the repo asset-free and deterministic, and it isolates the concept that matters (atlas + rects + frames) from the one that doesn't yet (file formats). Arc 4's resource system adds loading files through the same door.
- **Animation state is entity data, driven by a system.** `Entity` gains `sprite`, `frame`, `animTime`, and `flipX`. The movement system writes them: facing follows horizontal input (rendering flips by negative scale around a shifted origin), the walk clock advances only while moving (frame = animTime / 0.12s, mod 4), and stopping resets to the idle frame. Entities without a sprite (`sprites::NONE`) still draw as colour rectangles — the fallback keeps untextured things possible, which the inspector era will appreciate.

## Code built

- New `src/SpriteSheet.hpp/.cpp`: layout constants (frame size, cell positions, sprite ids) + `buildSpriteSheetImage()` with per-frame character painting and the crate. Added to GameLib in CMake (with the Lesson 8 re-configure).
- `Entity`: `sprite`, `frame`, `animTime`, `flipX` fields.
- `Game`: owns the sheet as an `sf::Texture` (built in the constructor, logged; failure falls back to rectangles with an error line). Spawns pass sprite ids. `update` drives facing + the walk clock. `render` cuts sprites from the sheet, scaled to entity size, flipped when needed; two-pass player-on-top drawing kept.
- Editor: the debug panel's player line now shows the live animation frame.

## What broke

One API break, caught by the compiler: `sf::Image::create()` no longer exists in SFML 3 — images are born sized, via the `sf::Image(Vector2u, Color)` constructor (or `resize`). Verified against the pinned 3.0.2 header before fixing, per the standing rule; the sheet builder now constructs the image directly. Everything else compiled first try.

## Verification

Built clean on the Linux rig (with the CMake re-configure the new `.cpp` requires). Three screenshots: `lesson-17-world.png` — the runtime world where the green rectangle used to be: a hooded traveller stands at centre, and both crates are now unmistakably wooden crates (planks, brace, nails). `lesson-17-walk-a.png` / `lesson-17-walk-b.png` — nearest-neighbour close-ups cropped mid-walk, 0.3 s apart while holding D: the stance differs between them (boots wide in stride in A, legs together in the passing pose in B) — the walk cycle advancing under game time, proven by the only honest evidence animation accepts: two frames that differ. One honest limit: the facing flip is implemented (negative scale, shifted origin) but this character is drawn left-right symmetric, so the flip is not visually provable yet — it will matter the day a sprite carries something asymmetric, like a sword.

## Explain-it check

_(Post-handover format: what actually changes between two frames of the walk — in memory terms? Why does the texture need to outlive every sprite cut from it? Why is the walk clock reset on stop instead of just paused? — the answers live in the Concept section above.)_

## Next step

Arc 4 opens — Lesson 18: scene save/load. The world (seed, entities, camera of the moment) serialises to a file of our own design and loads back identically; determinism from Arc 2 does the heavy lifting.
