# Devlog 069 — Scene Format v2

## Goal

Persist what the last two arcs created. Scene files move to **version 2**: each entity line appends its type, its script, its movement model, and its platform definition (patrol target, speed, one-way) — the Arc 3 fields that have been in-memory-only since Lesson 57, and the Arc 5 fields from Lesson 68. Version 1 files keep loading with the new fields defaulted; a future version is refused whole.

## Concept

- **Append, don't rearrange.** The v2 entity line is the v1 line with eight fields added at the end — the eleven fields a v1 reader understands keep their positions and meanings. Field *count* is the versioning inside the line: 11 parses as v1-with-defaults, 19 as v2, anything else is a line this engine doesn't understand and skips with a warning, per the loader's existing discipline (a scene loads whole or not at all; bad *lines* are skipped, bad *versions* refuse the file).
- **Definitions persist; runtime state doesn't.** A platform's patrol target and speed are its definition and are saved. Its `platformHome`, direction sign, and `frameDelta` are runtime state and are not — a loaded platform re-captures its home on first motion, exactly as a freshly spawned one does, so a saved scene and a rebuilt scene behave identically from the first frame. The same line is drawn for velocity and grounded: facts about the *world's construction* are saved; facts about *this run* are re-earned.
- **The type is saved as a name — and nothing checks it.** Loading a v2 scene does not require the type to exist in the registry: the entity keeps its fields as written, type name attached. A scene from a game whose types file you don't have still loads, renders, and plays — the registry is a convenience for *authoring*, not a gate on *loading*. (Lesson 68's stance, now load-bearing in the format.)

## Code built

- `src/Game.cpp` — `saveScene` writes `version=2` with the appended fields; `loadScene` accepts versions 1 and 2, parses 11- or 19-field entity lines, refuses anything newer.
- `src/Entity.hpp` — the platform comment updated: the definition persists, the state doesn't.
- `tests/tests_main.cpp` — the v2 check (suite now 48).

## Verification

- Harness: **48 passed, 0 failed** — a fully-loaded entity (type, script, platformer model, patrolling one-way platform) round-trips every new field exactly; a hand-written v1 file loads with defaults; a `version=3` file is refused.
- Probe: a real v2 file written to disk; its Crate line is the screenshot's source.
- Screenshot: `docs/shots/lesson-69-scene-v2.png` — the v2 entity line, annotated.
- Rig note: Xvfb died mid-suite once more during this lesson's first run; the standing recovery is now *restart the display immediately before each suite run*, which produced the green run recorded here.

## Explain-it check

_(Post-handover format: why append fields instead of writing a fresh layout for v2? Why is a platform's patrol target saved but its captured home not? Why doesn't loading check that an entity's type exists in the registry? — the answers live in the Concept section above.)_

## Next step

Lesson 70: scripts in the loop — the Game owns a ScriptHost, binds instances for every scripted entity when a scene loads (or a run starts), and calls their `onUpdate` inside `update()`, with a per-script error latch so a broken script logs once instead of every frame.
