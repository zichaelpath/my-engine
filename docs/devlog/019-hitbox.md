# Devlog 019 — The Hitbox Fix (Sprite Size Is Not Collision Size)

## Goal

A user-reported feel bug: "the movement hitbox feels really weird for this character." Reproduce it, measure it, fix it properly. This lesson interrupts the Arc 4 order (resources move to Lesson 20) because dogfood feedback outranks roadmap order — that is what dogfooding is for.

## Diagnosis

Reproduced by driving the runtime into the crate at (560, 380) and reading the editor panel: the player rests at **x=510 — exactly the crate's contact plane** (560 − 50). The collision math was never wrong. The *box* was wrong:

- Collision used the entity's full 50×50 sprite box.
- The traveller drawn inside that box occupies only the middle ~34 px — the sprite cell has ~8 px of transparent margin on each side, and the character's visual "presence" is his cloak and feet, not the square's corners.
- So the player stopped ~8 px before he looked like he should (invisible padding), caught invisible square corners on scenery his body visually cleared, and couldn't use gaps his body visually fit. Every symptom was the same cause: **the sprite's bounding box was doing a job it was never shaped for.**

## The fix

Collision and drawing now use different rectangles, as they always should have:

- `Entity` gains `hitboxOffset` + `hitboxSize` (a zero size means "the whole entity" — crates and everything else are unchanged).
- The player gets a **feet box**: offset (11, 28), size 28×22 — his legs and lower cloak, anchored to the bottom of the sprite. His head and shoulders can now overlap scenery visually (which even reads as depth), while his feet tell the truth about where he stands.
- `resolveCollisions` tests hitbox against hitbox (blocks keep their full rect); the world-bounds clamp holds the hitbox inside the world, so the hood may lean past the edge while the feet stop.
- Hitboxes are assigned by sprite type for now (`applySpriteHitbox`, also applied on scene load so F9 doesn't strip them). Per-instance hitbox editing belongs to the Arc 5 inspector, and the scene format will grow the fields then — stated now so nobody mistakes the shortcut for the design.
- The editor gains a **Show hitboxes** checkbox: red outlines of every entity's real collision box. A feel problem you can see is a feel problem you can tune.

## Verification

Built clean on the Linux rig. Before/after, same drive (down, then right into the crate), same crop (`docs/shots/lesson-19-before.png` / `lesson-19-contact.png`): before, the traveller halts with a visible strip of water between his hand and the crate; after, he stands pressed against the wood, hand at its face. The numeric witness: an F5 save after the after-drive records `Player|521|...` — x=521 is the feet-box contact plane (560 − 11 − 28), versus 510 for the old full box. And `docs/shots/lesson-19-editor.png` shows the overlay doing its job: a small red box at the traveller's feet, full red outlines on the crates.

One testing note, recorded honestly: the first editor verification drive overshot — under Xvfb the editor's frame rate had sagged to ~9 FPS, the held-key timing stretched, and the player approached below the crate's band and walked past it. The collision code was exonerated by the runtime runs at full frame rate (and by the saved x=521), but the episode is a fair warning about choreographed input tests at low FPS.

## Explain-it check

_(Post-handover format: why did a pixel-perfect collision system still feel wrong? Why is the hitbox anchored to the feet rather than centred? Which two rectangles does an entity now have, and which systems read which? — the answers live above.)_

## Next step

Lesson 20 — resources (the original Lesson 19, rescheduled): file-backed textures through a small resource manager, with caching and graceful fallback to the code-built sheet.
