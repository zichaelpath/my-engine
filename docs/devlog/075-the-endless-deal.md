# Devlog 075 — The Endless Deal

## Goal

Remove the ceiling: the climb must never run out of temple. Consumed chunks leave, fresh chunks arrive, and the run — and its score — continue indefinitely inside a world that never grows.

## Concept

- **The window, not the world, is what moves.** Growing the world upward was the naive answer and the wrong one: the tile grid, the movement clamps, and the camera all know a 1200-unit world, and an endless vector of tiles is an endless appetite. Instead the engine keeps a fixed *window* of live chunks. When the pressure fully consumes the bottom chunk, three things happen in one motion: the chunk's entities leave, **the entire world scrolls down by exactly that chunk's height**, and a fresh chunk is dealt into the freed space at the top. The window's footprint — top y, chunk count — is identical before and after. Content streams; geography stays home.
- **The scroll that moves nothing.** The shift moves every entity, every patrol target and patrol home, and the pressure line itself, all by the same amount. Relative to each other, nothing moved: the player stands exactly as far above the line as he did, on the same platform, mid-patrol unchanged. The only witness is a number — `m_scrollTotal`, the accumulated downward scroll — and the score adds it in: height = scrollTotal + (floor − feet). Forget the scroll and every recycle would rob the player of a chunk's worth of score; the harness samples the score on every frame after recycling begins and asserts it never falls.
- **The dealer works the whole run.** Lesson 73's dealer was a local Random that died with `buildAscent`. Now the run owns it — reseeded by `buildAscent`, drawn from by every refill, with the same no-back-to-back nudge — so a climb of ten thousand chunks is still one pure function of its seed. The initial deal is just the first five draws of an endless stream.
- **Honest staging, stated plainly.** The harness and probe climb with a *staged* bot — lifted 260 units every 45 frames — because what is under test is the window machinery, not the jumping (Lessons 59, 73 and 74 proved the jumping, with real input). The bot's lifts are teleports, labelled as such in the test, and both checks re-fetch the player entity every lift because recycling erases entities and a held reference would rot — the same stale-reference trap `update()` itself had to dodge when maintenance learned to erase mid-frame.
- **What does not stream yet.** The tile backdrop is a single carved column and does not scroll: the geology in the screenshot's two panels matches because the mountain is the window's frame, not its content. A scrolling or regenerating backdrop is visual work for a later pass; the platforms, the pressure and the score — the game — all stream today.

## Code built

- `src/Game.hpp` — `LiveChunk` records and the live-chunk deque, the run's dealer (`m_ascentRng`, last pick, next index), `m_scrollTotal` / `m_shaftLeft`, telemetry getters (`liveChunkCount`, `chunksDealtTotal`, `scrollTotal`), and the two new private workers.
- `src/Game.cpp` — `dealChunkOnTop()` (one draw, exact stacking, deque bookkeeping) now serves both `buildAscent` and refills; `maintainAscent()` (pop → scroll → floor retirement → refill, with the player index re-derived after erasures); `update()` calls maintenance as the line rises, re-fetches the player afterwards, and banks height with the scroll included; the shaft carve now runs the column's full height.
- `tests/tests_main.cpp` — the endless-deal check (suite now 54).

## Verification

- Harness: **54 passed, 0 failed** — a 3-chunk window at speed 60 streams for a simulated minute: chunks dealt climb past 6, the live count never leaves 3, the top returns to exactly 550 after every cycle, scrollTotal passes 600, the bot is never caught, and the score — sampled every frame from the first recycle on — never falls.
- Probe (shipped chunks, project seed): the engine's own log narrates the stream — *Gauntlet consumed; dealt Elevator (scroll 200) · Steps consumed; dealt Islands (400) · Islands consumed; dealt Steps (600) · Steps consumed; dealt Islands (800)* — ending at **9 dealt, 5 live, top back at exactly 150, height 1,728** in a 1,200-tall world, never caught.
- Screenshot: `docs/shots/lesson-75-endless-deal.png` — one fixed window of the world, before the stream and after four recycles: the platform arrangement has flowed, and the climber stands near the top of a climb taller than the world that contains it.
- One slip, owned: the harness's first tempo (pressure at 120 u/s) killed the staged bot at 0.42 s — before its first lift at 0.75 s. The machinery was innocent; the test's staging arithmetic was not. The tempo is now 60, with the reasoning written into the test where the next reader needs it.

## Explain-it check

_(Post-handover format: why scroll the world down instead of growing it upward? When a chunk is consumed, why must the pressure line scroll by the same amount as everything else? Why does the score need scrollTotal — what would go wrong without it? — the answers live in the Concept section above.)_

## Next step

Lessons 76–77: Temple Run becomes a shippable game — the runtime learns to run a project (title, climb, caught, score, restart), and the arc closes with Temple Run packaged as its own game, MyEngine 1.5.
