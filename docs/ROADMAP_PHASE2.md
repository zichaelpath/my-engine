# MyEngine — Phase 2 Roadmap

**Phase 1 is complete:** Lessons 1–43, nine arcs, MyEngine 1.0.0 (tagged `v1.0.0`) — an engine library, a dockable editor, a shipped game (Gold Rush), and a raw-OpenGL host. See `ROADMAP.md` and `CHANGELOG.md`.

**Phase 2 goal:** turn a *working* engine into a *proven* one. Phase 1 answered "can one developer build an engine and ship a game on it?" Phase 2 answers the harder questions: does it survive contact with its own author as a *player* (Arc 1), can it make games that *feel* good (Arc 2), can it do a second, different genre without hacks (Arcs 3–4), can games be *authored* in it rather than coded around it (Arcs 5–7), how far does the 3D go (Arc 8), and does it all hold together as a public 2.0 (Arc 9)?

**Estimated size:** ~46 lessons across nine arcs — deliberately the same depth as Phase 1 (43 lessons). Estimates are planning aids, not promises; Phase 1's arcs compressed and stretched as the work taught us things, and these will too. Devlog numbering continues from Lesson 44.

## Standing rules (carried over from Phase 1, unchanged)

- Every lesson ships in one commit: code + devlog + blog entry + README update + at least one screenshot.
- Every lesson is verified by *playing or running the thing*, never by "it compiles" alone; numbers come from the engine's own instruments (logs, checksums, panel readouts), not eyeballs.
- Determinism is sacred: same seed, same world — any system that touches generation keeps the checksum honest.
- Fakes get replaced, not layered over (the Lesson 39 rule).
- Formats are versioned and fail loudly rather than half-loading (the Lesson 41 rule).
- Zac's testing gates the phase: Arc 1 begins from *his* Phase 1 playtest findings, and any arc can be re-planned around what he finds.

**Progress — Nov 15, 2026:** Arc 1 is OPEN. Lesson 44 shipped its first two playtest findings: the left-facing sprite displacement (the flip branch's double shift — fixed and probe-verified) and the missing vendored `KHR/khrplatform.h` that broke the Windows build. Still ahead in this arc, in order: Zac's Windows rebuild + audio's first real-hardware listen, the test harness, CI, and 1.0.1.

## Arc 1 — First Contact (Lessons ~44–48)

*Harden 1.0 against its first real user: its author, on his own machine, as a player.*

- **Playtest harvest.** Every finding from Zac's Phase 1 testing — the Lesson 19 feet-hitbox feel verdict, the audio heard on real hardware for the first time (the build rig has no sound device; all playback timing is unverified until this arc), Windows build behaviour — triaged into fix lessons, in his priority order.
- **The Windows build, verified.** `docs/WINDOWS_BUILD.md` executed on the real machine; whatever differs from the walkthrough gets fixed in the walkthrough *and* the build, and the Windows Gold Rush package gets produced.
- **A test harness.** The Lesson 38 checksum probe grows up: a `MyTests` target that runs headless checks (world determinism checksum, scene save/load round-trip, project-file load/reject cases, config parsing) and prints pass/fail. One command, no display needed where possible.
- **CI.** GitHub Actions building Linux *and* Windows on every push, running the test harness on Linux. A red X on a commit is the cheapest code review there is — and it makes the portfolio repo read as professionally as it is.
- **1.0.1.** The fixes ship as a patch release: version bump, tag, changelog — the Lesson 42 machinery, exercised for real this time.

## Arc 2 — Game Feel (Lessons ~49–53)

*The systems that separate "it works" from "it feels good." Gold Rush is the test bed.*

- **Easing & tweens.** A small tween system in GameLib (ease curves as functions, tweened values over time) — the primitive under half of all juice.
- **Particles.** A CPU particle pool in Game: spawn/update/draw as one batched vertex array, emitters as data. Gold collection bursts are the first customer.
- **Camera feel.** Screen shake (trauma-based), hit-stop (brief time freeze on collection milestones), and smooth follow with lookahead — all as Camera/Game features the editor can toggle live.
- **Gold Rush, juiced.** The full pass over the shipped game: collection feedback, a win celebration, water splash particles (the splash *sound* has existed since Lesson 29; now it has a picture), UI pop on the HUD.
- **Feel is testable.** Each effect gets an editor toggle + a screenshot pair (off/on), so "juice" stays engineering, not vibes.

## Arc 3 — Platformer Physics (Lessons ~54–59)

*Gravity changes everything. This arc rebuilds movement around it — carefully, on top of the deterministic core.*

- **Gravity & grounded state.** Entities gain velocity; the player gains falling, landing, and a grounded flag — axis-separated resolution (Lesson 15's pattern) extended to the tile grid itself, not just entity-vs-entity.
- **Jumping, done properly.** Coyote time, jump buffering, variable jump height (release to cut) — the three tricks that make a jump feel right, each measured in milliseconds and tunable in the editor.
- **Tile collision.** Solid tiles from the tile grid (a solidity table beside the palette), so levels are terrain, not entity piles.
- **One-way platforms & moving platforms.** Pass-through-from-below tiles; platform entities that carry the player (position deltas applied to riders).
- **The physics panel.** Editor readouts for velocity, grounded, coyote/buffer timers — because feel you can't measure is feel you can't tune.

## Arc 4 — The Second Game (Lessons ~60–64)

*Proof of generalization: a complete platformer, authored in the editor, shipped mid-phase.*

- **The game.** A small precision-lite platformer (working title in planning; Zac names it): collect-the-things across hand-authored levels, built with Arcs 3+5 systems — no genre-specific hacks in GameLib; every gap the game exposes becomes an engine lesson or gets logged as a known limit.
- **Authored, not generated.** Levels are scenes (Lesson 18 format), laid out in the editor — the first real test of the editor as a *level* tool, feeding Arc 7's list.
- **Ship it.** Packaged like Gold Rush (its own `project.myengine`), versioned, clean-room verified. Working title for the release: MyEngine 1.5 — the engine grew a genre.
- *(Decision point for Zac, before this arc: platformer as planned, or a different second genre — see the end of this document.)*

## Arc 5 — Data-Driven Entities (Lessons ~65–69)

*Stop hardcoding what things are. Entity types become data the project owns.*

- **Type definitions.** A `types` section (its own file in the project): name, sprite, size, hitbox, solid, speed, behaviour flags. Spawning by type name replaces hand-built entities.
- **Behaviours as data.** A small set of composable behaviour components — patrol, chase, flee, collectible, hazard, carryable — implemented once in GameLib, attached by data. ECS-lite: components without the dogma, chosen because our entities are few and our author is one person.
- **Scene format v2.** Scenes store type + per-instance overrides; v1 scenes still load (the version gate from Lesson 41, honoured both ways — old files in, never silently mangled).
- **Editor integration.** Spawn menu lists project types; the Inspector edits overrides, not raw structs.

## Arc 6 — Levels, Campaigns & Saves (Lessons ~70–74)

*Games bigger than one screenful of world.*

- **Multi-scene projects.** The project file lists scenes; the Runtime moves between them (door/edge triggers, spawn points by name).
- **Campaign structure.** Gold Rush gains a multi-world campaign *or* the platformer gains its level sequence (whichever the Arc 4 decision makes natural): a level-select flow built from shared UI widgets (below).
- **Shared UI widgets.** Labels, buttons, and panels as engine objects both hosts can use — the Runtime's hand-rolled title screen (Lesson 33) is refactored onto them, so menus stop being per-game code.
- **Save files.** A versioned `save.dat` (same text family): campaign progress, best times (Gold Rush finally keeps its records), settings that persist per player rather than per machine.

## Arc 7 — The Editor Grows Up (Lessons ~75–80)

*From inspector to authoring tool — the arc Arc 4's level-building pain writes the spec for.*

- **Terrain painting.** Brushes that paint tile types onto the grid (with the determinism contract stated plainly: painted tiles are *edits stored in the scene*, layered over the generated base — generation stays pure).
- **Palettes.** Tile palette + entity-type palette panels; click-to-arm, paint-to-place, right-click erase.
- **Scene manager.** Create/rename/delete scenes in the project from inside the editor; the project file stays the source of truth on disk.
- **Grid & snapping controls.** Snap sizes, tile-aligned placement for level work, world-border tools.
- **Layout presets.** Saved panel layouts (the editor's `imgui.ini` becomes a named, choosable thing — a debt noted since the Phase 1 screenshots).

## Arc 8 — 3D, Second Season (Lessons ~81–86)

*MyGL grows from demo to renderer. Zac's parallel LearnOpenGL study feeds this arc directly.*

- **Specular light.** Blinn-Phong highlights join the Lambert sun; materials get a shininess term (water shines first).
- **Shadows.** Shadow mapping: a depth pass from the sun, terrain that shades itself — the single biggest visual step left in the 3D host.
- **Meshes as assets.** A `Mesh` class (VAO/VBO/EBO owned properly) and an OBJ loader, so models stop being hand-typed arrays; the crate becomes a loaded model.
- **Sky & atmosphere.** A gradient skybox/clear and distance fog — the heightfield stops floating in void.
- **Stretch (honestly flagged):** a read-only 3D viewport panel inside the editor, rendering the scene through MyGL's renderer. Only if Arcs 1–7 land on schedule; cut without guilt otherwise.

## Arc 9 — Ship 2.0 (Lessons ~87–90)

- **Performance pass.** A profiling readout (frame-time graph in the editor, draw-call and triangle counters in MyGL), one measured optimization chosen by data, before/after numbers in the devlog.
- **The public question.** *(Decision point for Zac:)* the repo goes public as the portfolio piece, or stays private with a public showcase page. Either way: a release page with screenshots, the two games, and the story.
- **2.0.0.** Both games repackaged on the 2.0 engine, changelog, tag — and the Phase 2 retrospective devlog: what the engine is now, what it still isn't, and what Phase 3 would be about (current honest candidates: a scripting layer, networking, or the editor's own 3D view — in that order of likelihood).

## Decisions Zac owns (needed before the arc named)

1. **Before Arc 4:** the second game's genre — platformer (planned; it forces the most reusable engine growth) or something else he names.
2. **Before Arc 9:** public repo vs private + showcase.
3. **Any time in Arc 1:** his Phase 1 playtest findings *are* Arc 1's spec — the arc re-plans around them, not vice versa.

*Planned Nov 15, 2026, the day Phase 1 shipped. Same author, same rules, same standard: if it isn't verified by playing it, it isn't done.*
