# Devlog 050 — Particles: The Splash Gets Its Picture

## Goal

Arc 2's second primitive: a particle system in GameLib — a CPU pool with emitters as data — proven on the two effects Gold Rush has been waiting for. Collecting gold now bursts. Wading into water, which has *sounded* like a splash since Lesson 29, finally *looks* like one.

## Concept

- **Emitters are data, particles are labour.** An `Emitter` is a plain description (position, count, colour, speed range, direction + spread, life range, size range, gravity) and `spawn()` rolls it into live particles. The two effects in this lesson are the same code wearing different numbers: the gold burst is a full-circle spray with mild gravity; the splash is a narrow upward cone with heavy gravity — a fountain. When a future game wants rain, sparks, or jet exhaust, it will write numbers, not systems.
- **A flat pool, swap-with-last.** Particles live in one vector, die by swapping with the last (the gold dots' own trick since Lesson 32), cap at 1024 — beyond capacity a burst clips instead of growing unbounded. Drawing is one `sf::VertexArray` of triangles (SFML 3 has no quad primitive), alpha fading with the life fraction: born solid, dies clear.
- **Cosmetic randomness stays in its lane.** Spawn rolls draw from the Random class's **"fx" stream**, which is derived from the seed independently of every generation stream (Lesson 10's design paying its rent). A burst can therefore never change the world it decorates — and this isn't a hope: the harness's 1.0 grid-checksum check runs in the same suite and still passes.
- **One call site shape.** `Game::spawnParticles(emitter)` is the single door: the collection code calls it, the splash edge-detector calls it, the harness calls it, and Arc 5's scripts will call it. Feel effects that route through one public API are feel effects that stay testable.

## Code built

- `src/Particles.hpp/.cpp` — `Particle`, `Emitter`, `ParticleSystem` (spawn/update/draw, 1024 cap).
- `src/Game.*` — pool member, `spawnParticles` + `particleCount`, `particlesEnabled` toggle (the feel A/B switch this arc keeps promising), gold-burst on collection, splash fountain on the water-entry edge, particle advance in `update`, draw after entities and under the debug overlays.
- `CMakeLists.txt` — `Particles.cpp` joins GameLib (Lesson 8's rule).
- `tests/tests_main.cpp` — three new checks (suite now 18).

## Verification

- Harness: **18 passed, 0 failed.** New checks: spawn fills the pool and time empties it; the toggle silences spawns; two systems fed the same seed produce identical bursts (velocities and lifespans, exactly). The pre-existing grid-checksum check passing in the same run is the fx-stream isolation proof.
- Render probe through the public API: both emitters fired, **48 particles alive** in the rendered frame (26 + 22, none yet expired at 0.12 s — the arithmetic agrees).
- Screenshot: `docs/shots/lesson-50-particles.png` — the gold burst mid-flight (upper right) and the splash fountain rising at the player's feet (lower left).
- Coverage honesty: the harness drives `spawnParticles` directly; the collection and splash call sites are one-liners over it, exercised in real play (and in Zac's hands tonight) rather than by synthetic keyboard input the headless rig can't honestly provide.

## Explain-it check

_(Post-handover format: why must cosmetic randomness use its own stream? Why does a splash use a narrow cone and heavy gravity while a burst uses a full circle and light gravity — same code, different numbers? Why cap the pool instead of letting it grow? — the answers live in the Concept section above.)_

## Next step

Lesson 51: camera feel — trauma-based shake (a hard landing's and a bump's rattle), hit-stop on the final gold, and the follow camera's last hard edges sanded.
