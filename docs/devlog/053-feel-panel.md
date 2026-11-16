# Devlog 053 — Feel Is Data: The Editor Panel, and Arc 2 Closes

## Goal

Arc 2's closing lesson makes the whole feel layer *adjustable*: every constant Lessons 49–52 tuned into the code becomes a field in one struct, `Game::Feel`, and the editor grows a **Feel panel** that edits it live — sliders, a master switch, trigger buttons, and live readouts. The A/B the arc promised (all feel off vs all feel on) is now one checkbox.

## Concept

- **Constants are decisions; data is a dial.** The pop's 0.35 s, the shake's 14 units, the streak's 1.2 s window — each was a choice made while building its lesson. As literals scattered through `Game.cpp` they were frozen; as fields in one struct they are a *personality* a game can restyle without recompiling. The defaults in the struct are exactly the shipped numbers, so nothing about Gold Rush's feel changed today — what changed is who can change it.
- **One struct, three readers.** The game code reads `feel.*` everywhere the literals lived. The harness pins behaviour against it (master switch off → no pop, no trauma, no stop, no particles, no shake; a 5 s streak window holds a streak the shipped 1.2 s would forget; burst size 0 fires rings but no particles). The editor panel writes it. Same data, three hands — no drift between what the tool shows and what the game does.
- **A panel is a workbench when it can fire the events.** Alongside the sliders: **Fire: pickup / shake / win** buttons that call the game's own public hooks (`onGoldCollected`, `addTrauma`, `onRunWon`) — the same calls the collection loop makes. You don't tune a burst by imagining it; you press the button and watch it, change a number, press again.
- **Readouts close the loop.** Trauma, streak, live particle and ring counts, printed in the panel — the engine's instruments (Lesson 44's lesson: navigate by instruments, not eyeballs) inside the tool.

## Code built

- `src/Game.hpp` — `struct Feel` (master switch, particles switch, pop duration, shake max, trauma decay, win hit-stop, streak window, burst base/per-streak) as a public `feel` member; the standalone `particlesEnabled` flag folded into it.
- `src/Game.cpp` — eleven call sites moved from literals/flags to `feel.*`; ring creation gated by the same switches as particles (the master switch now silences state, not just drawing).
- `src/main.cpp` — the Feel panel: 2 checkboxes, 5 sliders, 3 Fire buttons, live readouts.
- `tests/tests_main.cpp` — the Lesson 50 toggle check repointed at the struct; two new checks (suite now 26).

## Verification

- Harness: **26 passed, 0 failed** — including every Lesson 49–52 check passing *unchanged* against the struct defaults (proof the refactor didn't retune the game), plus the master-switch and tuning-reaches-code checks.
- Editor: launched headless; ImGui's own saved layout records `[Window][Feel]` at 280×257 — the panel exists, sized like the workbench it is. Screenshot: `docs/shots/lesson-53-feel-panel.png` — the editor with the Feel panel open at its default berth (top-left), Procgen its neighbour.
- Arc 2's closing A/B, stated for the record: master switch off reproduces the pre-Arc-2 game (asserted by the harness, not eyeballed); on is the game Lessons 49–52 built.

## Explain-it check

_(Post-handover format: why keep the defaults in the struct identical to the literals they replaced? Why do the Fire buttons call the game's public hooks instead of poking particle state directly? What breaks if the panel edited its own copy of the numbers? — the answers live in the Concept section above.)_

## Next step

Arc 3 — Platformer Physics: gravity and groundedness first (Lesson 54), giving entities a second movement model beside the top-down one — the foundation the vertical Temple Run will climb on.
