# Devlog 052 — Gold Rush, Juiced

## Goal

Arc 2's systems stop being demos and become a game: every primitive from Lessons 49–51 aimed at Gold Rush at once. A **collection streak** that escalates bursts, **shockwave rings** where things happen, a **drip** when you step out of water, and a **win sequence staged in beats** instead of everything firing on the same frame.

## Concept

- **Streaks make flow visible.** Pickups within 1.2 s of each other increment a streak; the burst grows with it (10 + 2×streak, capped). A flowing run across a gold field now *looks* different from plodding between single dots — the game rewards momentum with spectacle, which is most of what "juice" means. The window forgetting (streak → 0 when it lapses) matters as much as the escalation: a reward that never resets stops being information.
- **Rings are punctuation.** A burst says "particles!"; an expanding, fading outline says "*here* — something happened *here*." Rings ride the particle toggle (they are the same layer), expand on `ease::outCubic` (fast birth, gentle arrival — Lesson 49's curves earning their keep), and take 0.28 s to live.
- **A win is a sequence, not a frame.** Beat one: the freeze + rattle (Lesson 51's hit-stop and trauma, triggered together). Beat two: after the stop releases the world, a fountain over the player — nine-particle bursts every 80 ms for 0.7 s, alternating gold and pale gold, positions rolled on the fx stream. Beat three: a wide slow ring closes it. The staging works because the fountain clock sits *below* the hit-stop gate in `update` — it cannot start early, by construction rather than by care.
- **The world outlives the run.** The Runtime now keeps updating the world behind the Won screen (the fountain needs an audience of frames), and Game ignores input once won — the player takes his bow and stands still while the world celebrates around him. Previously the Runtime stopped calling `update` at the win, which would have frozen any staged effect mid-air: sequencing is a cross-host concern, and this is where it bit.
- **Hooks, not hacks.** The collection loop calls two public events — `onGoldCollected(at)` and `onRunWon()` — and nothing else changed about how dots are collected. Public hooks are what let the harness fire the exact code the game fires (and what Arc 5's scripts will listen to).

## Code built

- `src/Game.*` — streak state + window, `Ring` pool (expand/fade/die), `onGoldCollected` (burst sized by streak + ring), `onRunWon` (stop + trauma + wide ring + fountain arming), the fountain clock, water-exit drip on the tile edge detector, input ignored after the win, rings drawn over particles.
- `src/runtime_main.cpp` — `update` continues on the Won screen.
- `tests/tests_main.cpp` — three new checks (suite now 24).

## Verification

- Harness: **24 passed, 0 failed.** Streak: first pickup fires exactly 12 particles + 1 ring; the second, inside the window, fires 14 (26 total — the crescendo, counted); after 1.3 s the streak is forgotten. Win: `onRunWon` leaves the game hit-stopped with trauma up, a ring open, and *zero* fountain particles — then one update past the stop and the fountain is flowing (staging, asserted in order). Drip: driven honestly through the real world — settle on grass, wade into a real water tile, let the splash die, step out: **exactly +6 particles**.
- Probe: two pickups in streak rendered mid-flight — streak 2, 26 particles, 2 rings alive in the frame.
- Screenshot: `docs/shots/lesson-52-juice.png` — the crescendo burst and its expanding ring beside the player.

## Explain-it check

_(Post-handover format: why does the streak need a window that forgets? Why does the fountain clock have to sit below the hit-stop gate to stage correctly? Why did the Runtime have to change for a Game-side effect? — the answers live in the Concept section above.)_

## Next step

Lesson 53 closes Arc 2: the editor's Feel panel — every effect in the arc toggleable and tunable live, so the whole layer can be A/B'd (all off vs all on) from inside the tool it ships in.
