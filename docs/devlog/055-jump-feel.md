# Devlog 055 — Jumping, With the Forgiveness Built In

## Goal

The platformer learns to jump — not the bare `velocity.y = -jumpSpeed` of a tutorial, but the full feel kit that separates a jump players love from one they tolerate: **coyote time**, **jump buffering**, **variable jump height**, and a **landing that announces its weight**. Underneath them all, the lesson's real foundation: an **input seam** that lets anything — tests today, scripts and AI later — drive a player without a keyboard.

## Concept

- **The seam comes first.** `update()` read `sf::Keyboard` directly, which made every behaviour behind it untestable headlessly. Now input arrives as an `InputState` (move + jump): the keyboard fills it by default; `Game::setInput` overrides it. The override is not a test hack — it is the same door a Lua script (Arc 5), a replay system, or an AI driver will walk through, which is why it lives in Game's public API with the edge detection (pressed = held now, not held last frame) computed *inside*, identically for both sources.
- **Two grace timers, opposite sides of the same moment.** Coyote time (0.10 s) forgives a press just *after* the ground vanishes — you ran off the crate, pressed a hair late, and the game pretends you were still standing. The buffer (0.12 s) forgives a press just *before* landing — you meant to jump the instant you touched down, and the game remembers. Neither is a fudge; both model what the player *intended* better than the frame-exact truth does. They also cover each other's blind spot: a press on the very first standing frame, when `grounded` isn't true yet, is saved by the buffer.
- **Variable height is a cut, not a curve.** Release the button while still rising and the upward velocity is multiplied once by 0.45. Held jump apex: ~102 units. Tapped: ~35. One multiplication, and the same button produces hops and leaps — the cheapest expressive range in platforming.
- **Landings have weight classes.** A fall that ends above 480 u/s lands *heavy*: dust at the feet, a whisper of trauma (0.08). Below that, silence — a hop's return and a step down shouldn't announce themselves, or the heavy landing stops meaning anything.

## Code built

- `src/Game.hpp` — jump numbers as data (`jumpSpeed` 620, `coyoteTime`, `jumpBuffer`, `jumpCut`); the `InputState` seam (`setInput`/`clearInput`); timer state.
- `src/Game.cpp` — input read through the seam (jump = Space, plus W/Up for platformer players); the jump block in the platformer branch (timers, fire, cut); the heavy-landing effects in the grounded bookkeeping, reading fall speed before it is zeroed.
- `tests/tests_main.cpp` — three new checks, all driven through the seam (suite now 31).

## Verification

- Harness: **31 passed, 0 failed.** Held jump launches (velocity < −100 the first frame), apexes below y=40 from a y=100 rest, and returns to exactly y=100; the tapped jump apexes at least 25 units lower. Coyote: a press 3 frames after walking off the crate jumps; the identical press at 12 frames falls. Buffer: a press 4 frames before touchdown fires on the first grounded decision after it.
- Probe strip: stand y=100.0 → rise y=72.0 (vel −530) → apex y=−1.7 (a ~102-unit rise, against the 620²/2·1800 ≈ 107 the physics predicts — the difference is discrete frames, honestly accounted) → land y=100.0.
- Screenshot: `docs/shots/lesson-55-jump.png` — the four instants of that arc.

## Explain-it check

_(Post-handover format: why does the buffer save a press on the first standing frame, when coyote can't? Why is variable height a single multiplication instead of weaker gravity while the button is held? Why must the landing check read the fall speed before the grounded code zeroes it? — the answers live in the Concept section above.)_

## Next step

Lesson 56: tiles become collision — a solidity table for the tile grid, so platformer ground can be *terrain* (and a climber's walls can be rock), not only crates, blocks, and the world's floor.
