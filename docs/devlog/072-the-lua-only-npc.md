# Devlog 072 — The Lua-Only NPC: Arc 5 Closes

## Goal

Arc 5's proof, end to end: an NPC whose *entire* behaviour lives in a `.lua` file in a project's `scripts/` folder — no patrol code anywhere in the engine. Ship the script in this repo's own project with a demo scene, prove the behaviour in the harness, and prove the shipped file itself with a probe.

## Concept

- **The engine knows nothing about patrols.** There is no `PatrolComponent`, no behaviour enum, no C++ that has heard of a beat, a range, or turning around. `scripts/patrol.lua` holds two constants (`SPEED 60`, `RANGE 120`) and one `onUpdate` that reads its position, steps, clamps exactly on the bound, and turns. Everything it uses arrived in earlier lessons on purpose: the API (67), the type that names it (68), the scene field that carries it (69), the loop that runs it (70), the reload that refreshes it (71). This lesson writes *no engine code* — that absence is the deliverable.
- **Home is captured, not configured.** The script remembers its starting x on its first frame and patrols around it. Place the NPC anywhere — in the editor, in a scene file, from a spawner — and its beat centres there. The same capture is what makes hot reload graceful: a reloaded script re-homes where the NPC stands, so an edit never teleports anyone.
- **A twinship, stated honestly.** The harness cannot read the repo's files (it runs from a scratch folder, deliberately), so its check embeds the patrol logic with `RANGE` parameterised — which is also what lets it prove the second clause, that changing the constant changes the beat. The *shipped* file is then verified separately by the probe, run in the real project against the real `scripts/patrol.lua`. Two instruments, two jobs, neither pretending to do the other's.
- **The demo is a file, not a mode.** `scenes/patrol-demo.scene` is an ordinary v2 scene whose PatrolNpc line carries `scripts/patrol.lua`. Open it in the editor, press play: the guard walks. Nothing about the demo is special-cased anywhere — which is precisely what makes it a proof.

## Code built

- `scripts/patrol.lua` — the NPC, complete: constants, home capture, the clamped turn.
- `scenes/patrol-demo.scene` — a v2 scene (written by the probe through `saveScene`) placing PatrolNpc at (480, 420) with the script attached.
- `tests/tests_main.cpp` — the Lua-only NPC check (suite now 51).
- No engine changes. None were needed; that is the point of the arc.

## Verification

- Harness: **51 passed, 0 failed** — a Guard spawned from a type whose only distinction is its script walks two seconds out to exactly home + 120 and two seconds back home; with `RANGE` edited to 60 in the file, a fresh run never strays past home ± 60 and still reaches its bound. The constant rules the beat.
- Probe (the shipped file, the real project): home **480.0**, max **600.0** — home + RANGE, exactly — back to **480.0** after four seconds; the demo scene saved through the engine's own writer; a fresh Game loading that scene reports **1 binding** with no explicit bind — Zac's open-and-play flow, verified headlessly.
- Screenshot: `docs/shots/lesson-72-lua-npc.png` — the magenta NPC at home, mid-beat, and on the bound.

## Explain-it check

_(Post-handover format: why does the patrol capture its home on the first frame instead of taking it from the scene file? Why does the harness embed the script instead of reading the shipped file — and what covers the shipped file instead? Why is "no engine code changed" the strongest possible result for this lesson? — the answers live in the Concept section above.)_

## Arc 5, closed

The second pillar stands. A game's behaviour now lives in that game's files: Lua embedded by hand (66), an API a script can hold (67), types as data (68), scenes that persist all of it (69), a loop that runs it (70), reloads that refresh it (71), and an NPC that proves it (72). Next: Arc 6 — the second game, the vertical Temple Run, authored with types and scripts from the first line.
