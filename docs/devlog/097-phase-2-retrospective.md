# Devlog 097 — Phase 2 in the Mirror: What Ten Arcs Actually Built

*Arc 10 (Ship 2.0) opens with the retrospective the roadmap promised:
before the version number moves, an honest account of the phase.*

## The phase in one paragraph

Phase 1 (Lessons 1–43) built an engine that could make **a game**:
Gold Rush, in an editor, with a runtime and a 3D host beside it.
Phase 2 (Lessons 44–94, plus the 95–96 playtest hotfixes) built an
engine that can make **games**: a second game (Temple Run) that shares
no code with the first, games written in Lua instead of C++, projects
that live in their own folders, campaigns that travel between scenes,
saves that survive version bumps, an editor with a second game view
and a 3D eye, and a 3D renderer that grew from a lit heightfield into
specular, shadows, meshes, sky, and fog. The releases tell the same
story in numbers: 1.5.0 (the second game), 1.6.0 (campaigns and
saves), 1.7.0 (the editor grows up), 1.8.0 (the 3D second season) —
and a harness that grew from 10 checks to **69**, every one of them
earned by a feature or a bug.

## Arc by arc, in one breath each

- **Arc 1 — First Contact (44–48):** the harness and CI were built
  *before* the features they would judge; the flip regression they
  caught in their first week is why.
- **Arc 2 — Game Feel (49–54):** particles, streaks, shockwaves, a
  staged win — the phase's thesis in miniature: feel is systems, and
  systems are testable.
- **Arc 3 — Platformer Physics (55–59):** gravity, coyote time, jump
  buffering, one-way and moving platforms. The tile grid became
  collision, and the engine stopped being top-down-only.
- **Arc 4 — The Project Is a Place (60–64):** the project became a
  folder with a card in it; the editor learned to browse and adopt
  one. This arc is the one Phase 3 now returns to — see below.
- **Arc 5 — Scripting (65–71):** Lua 5.4.7, hand-bound, no library
  between the engine and the language. Entity types, scene format
  v2, hot reload, and an NPC whose entire behaviour is a script —
  the North Star's second pillar, standing.
- **Arc 6 — The Second Game (72–77):** Temple Run proved the engine
  by the only proof that counts: a game its author designed, built
  from types and scripts, packaged like the first.
- **Arc 7 — Levels, Campaigns & Saves (78–82):** multi-scene
  projects, doors that latch, a UI widget kit, and a versioned
  `save.dat` that refuses the future politely.
- **Arc 8 — The Editor Grows Up (83–88):** terrain painting as
  differences, a pop-out Game View, a Levels panel, play-test
  travel, a second game view. The editor stopped being one window
  with panels and became a desk.
- **Arc 9 — 3D, Second Season (89–94):** specular, shadow mapping,
  a Mesh class and OBJ loader, sky and fog, and the editor's 3D
  View — the last one only possible because the terrain builder and
  shader compiler were extracted into shared code first.
- **The hotfixes (95–96):** the playtest gate's first catches. Two
  Windows build walls that the Linux rig structurally cannot see —
  recorded in their own devlogs, and the reason the gate is now a
  permanent institution rather than a phase.

## What the phase changed about the engine's nature

Three shifts, in hindsight. **From game to platform:** everything
after Arc 5 is shaped by the question "could a stranger's game use
this?" — types, scripts, campaigns, and saves are all answers.
**From hosts to shared code:** Arc 9's extractions (TerrainMesh,
GLProgram) mean the 3D the editor shows and the 3D the host flies
are the same code, not cousins. **From my verification to the
author's:** the harness measures what a machine can measure; the
playtest gate exists because layout, feel, and Windows headers are
not among them. Phase 2 is the phase the engine got a second pair
of eyes.

## The honest misses

The packaging detour of Lesson 94 (38 minutes building SFML in a
stray folder because of one wrong argument) is in its devlog, not
hidden. The Windows walls of 95–96 sat in the tree from Lesson 93
until the author's first build found them — Linux verification,
however thorough, is one platform's truth. GUI click-paths remain
the least-verified surface: several editor flows are proven by
their state (layouts saved, scenes written) rather than by driven
clicks, and are listed in the playtest gate for exactly that
reason. And the front door — first-launch layout, creating a
project, adding a file — was still engine-shaped rather than
rookie-shaped when the author opened it. That list became
Phase 3 within the hour, which is the gate working as designed.

## What Phase 3 inherits

A capable engine with a proven second game, a scripting layer, a
3D eye, and a permanent playtest gate — and a roadmap
(`docs/ROADMAP_PHASE3.md`) aimed at the front door: default
layouts, projects that are not the engine, files you can touch,
undo. Phase 2 made the engine able. Phase 3 makes it welcoming.
