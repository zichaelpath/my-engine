# MyEngine — Phase 2 Roadmap

**Phase 1 is complete:** Lessons 1–43, nine arcs, MyEngine 1.0.0 (tagged `v1.0.0`) — an engine library, a dockable editor, a shipped game (Gold Rush), and a raw-OpenGL host. See `ROADMAP.md` and `CHANGELOG.md`.

**Phase 2 goal:** turn a *working* engine into a *proven* one. Phase 1 answered "can one developer build an engine and ship a game on it?" Phase 2 answers the harder questions: does it survive contact with its own author as a *player* (Arc 1), can it make games that *feel* good (Arc 2), can it do a second, different genre without hacks (Arcs 3–4), can games be *authored* in it rather than coded around it (Arcs 5–7), how far does the 3D go (Arc 8), and does it all hold together as a public 2.0 (Arc 9)?

**Estimated size:** ~46 lessons across nine arcs — deliberately the same depth as Phase 1 (43 lessons). Estimates are planning aids, not promises; Phase 1's arcs compressed and stretched as the work taught us things, and these will too. Devlog numbering continues from Lesson 44.

## What "complete" means (Zac's north star, Nov 15, 2026)

Phase 2's destination, in the author's words: *a complete engine* — "a standard engine feel to where any rookie game developer can download it and create a video game, whether that's with or without procedural generation." Three pillars follow from that, and the arc order below was revised the same day to serve them:

- **A file system.** A project is a *place* — a folder the editor shows you. Files (scenes, scripts, assets) are visible, creatable, and openable from inside the editor, not folklore you manage in Explorer. (Arc 4)
- **Scripting per game.** A game's behaviour lives in *that game's* script files, not in engine C++ written by the engine's author for one title. The engine ships the stage and the API; each game brings its own play. (Arc 5, proven by Arc 6)
- **Multiple windows if necessary.** The editor is not trapped in one rectangle: the game view (and later the script editor) can live in their own OS windows when the work wants the room. (Arc 8)

Procgen stays what it has always been here: a foundation a game *may* build on — never a toll every game must pay. A rookie who wants a hand-made platformer and nothing else should never have to know the noise library exists.

## Standing rules (carried over from Phase 1, unchanged)

- Every lesson ships in one commit: code + devlog + blog entry + README update + at least one screenshot.
- Every lesson is verified by *playing or running the thing*, never by "it compiles" alone; numbers come from the engine's own instruments (logs, checksums, panel readouts), not eyeballs.
- Determinism is sacred: same seed, same world — any system that touches generation keeps the checksum honest.
- Fakes get replaced, not layered over (the Lesson 39 rule).
- Formats are versioned and fail loudly rather than half-loading (the Lesson 41 rule).
- Zac's testing gates the phase: Arc 1 begins from *his* Phase 1 playtest findings, and any arc can be re-planned around what he finds.

**Progress — Nov 15, 2026:** Arc 1 is OPEN. Lesson 44 shipped its first two playtest findings: the left-facing sprite displacement (the flip branch's double shift — fixed and probe-verified) and the missing vendored `KHR/khrplatform.h` that broke the Windows build. Lesson 45 shipped the test harness (MyTests — ten checks, incl. the 1.0 world checksum and the flip regression); Lesson 46 shipped CI (GitHub Actions: Linux + Windows build and test on every push, both green first run). Still ahead in this arc: Zac's Windows rebuild + audio's first real-hardware listen, and 1.0.1. **Arc 2 has opened in parallel** (Lesson 49: easing & tweens, the spawn-in pop; Lesson 50: particles — gold bursts and the splash's picture) — it does not touch the 1.0.1 gate, which ships on Zac's word.

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

## Arc 4 — The Project Is a Place (Lessons ~60–65)

*The file-system pillar. A project becomes a folder the editor can see, browse, and manage — the Asset Browser this engine's plan has promised since Nov 13, 2026.*

- **Project folders.** A project is a directory containing `project.myengine`; the editor opens a *folder*, and everything the game owns lives under it (scenes/, scripts/, assets/). Gold Rush migrates to the layout without changing its identity.
- **The FileSystem panel.** A dockable tree of the project folder: scenes, scripts, assets, each with its icon and kind; click to select, double-click to open (scene → load, script → script editor when Arc 5 lands, asset → preview).
- **File dialogs.** Open Project, New Project (name + folder + starter layout), Save Scene As — real OS dialogs or an ImGui equivalent, decided by what SFML/porting makes honest; no more "the file is wherever the working directory was" (Lesson 44's checkerboard was this arc knocking early).
- **Import.** Drag or pick an image/sound file and it is *copied into* the project's assets and enters the resource caches by project-relative path — the end of CWD-relative asset luck.
- **Scene files as citizens.** Create/rename/duplicate/delete scenes from the panel (absorbs Arc 7's old scene-manager item); the project file lists them; the editor's scene controls and the panel never disagree.

## Arc 5 — Scripting (Lessons ~66–72)

*The scripting pillar: behaviour belongs to the game, not the engine. (Language decision is Zac's — see Decisions; the plan is written for the recommended option.)*

- **A language, embedded by hand.** Lua, built from source via FetchContent and bound through its plain C API — no binding framework, the same own-the-stack choice as the hand-written GL loader over GLAD. A `ScriptHost` in GameLib owns the VM.
- **Scripts are project files.** `.lua` files under the project's `scripts/` (Arc 4's layout), attached to entities by name — in the type data (below) and per-instance in scenes (scene format grows a `script=` field, version-gated like Lesson 41 taught).
- **An engine API worth scripting against.** v1 surface, deliberately small: entity get/set position, move with collision, spawn/despawn, input queries, play sound, log, the run's named RNG stream, tile queries. Everything a script can do, a test can check.
- **Lifecycle.** `on_spawn`, `on_update(dt)`, `on_collect`-style events as the game defines them; scripts reload from the editor without restarting (edit → save → the world picks it up), because iteration speed is the entire point.
- **Data-driven types, completed.** Arc 5's original content lands here as the data half: entity *types* defined in the project (name, sprite, size, hitbox, solid, speed) — types say WHAT a thing is, scripts say HOW it behaves. Scene format v2 stores type + overrides + script.
- **Proof scene.** A test scene whose NPCs patrol, chase, and flee — written entirely in Lua, zero genre code in GameLib.

## Arc 6 — The Second Game (Lessons ~73–77)

*Proof of generalization, now with the full stack: a complete platformer, authored in the editor, its behaviours written in its own Lua scripts.*

- **The game.** A small precision-lite platformer (Zac names it): collect-the-things across hand-authored levels, built with Arc 3 physics + Arc 4's project layout + Arc 5's types and scripts — no genre-specific hacks in GameLib; every gap the game exposes becomes an engine lesson or a logged known limit.
- **Authored, not generated.** Levels are scenes, laid out in the editor — the procgen foundation deliberately unused, per the north star: the engine serves both kinds of game.
- **Ship it.** Packaged like Gold Rush (its own project folder), versioned, clean-room verified. Working title for the release: MyEngine 1.5 — the engine grew a genre *and* a scripting layer.
- *(Decision point for Zac, before this arc: the platformer's name and fantasy — mechanics are planned, identity is his.)*

## Arc 7 — Levels, Campaigns & Saves (Lessons ~78–82)

*Games bigger than one screenful of world.*

- **Multi-scene projects.** The project file lists scenes; the Runtime moves between them (door/edge triggers, spawn points by name).
- **Campaign structure.** The platformer gains its level sequence (and Gold Rush a multi-world campaign if it still earns it): a level-select flow built from shared UI widgets (below).
- **Shared UI widgets.** Labels, buttons, and panels as engine objects both hosts can use — the Runtime's hand-rolled title screen (Lesson 33) is refactored onto them, so menus stop being per-game code.
- **Save files.** A versioned `save.dat` (same text family): campaign progress, best times (Gold Rush finally keeps its records), settings that persist per player rather than per machine.

## Arc 8 — The Editor Grows Up, Windows and All (Lessons ~83–88)

*From inspector to authoring tool — and out of its single rectangle, per the third pillar.*

- **Terrain painting.** Brushes that paint tile types onto the grid (determinism contract stated plainly: painted tiles are *edits stored in the scene*, layered over the generated base — generation stays pure).
- **Palettes.** Tile palette + entity-type palette panels; click-to-arm, paint-to-place, right-click erase.
- **Windows, plural.** The Game View pops out into its own OS window (a second `sf::RenderWindow` rendering the same world — play on one screen, inspect on the other), and the script editor follows when Arc 5's editing surface wants the room. ImGui multi-viewport docking is the stretch goal, honestly flagged: ImGui-SFML's viewport support is partial, and the pop-out windows land first either way.
- **Grid & snapping controls.** Snap sizes, tile-aligned placement for level work, world-border tools.
- **Layout presets.** Saved panel layouts (the editor's `imgui.ini` becomes a named, choosable thing — a debt noted since the Phase 1 screenshots).

## Arc 9 — 3D, Second Season (Lessons ~89–94)

*MyGL grows from demo to renderer. Zac's parallel LearnOpenGL study feeds this arc directly.*

- **Specular light.** Blinn-Phong highlights join the Lambert sun; materials get a shininess term (water shines first).
- **Shadows.** Shadow mapping: a depth pass from the sun, terrain that shades itself — the single biggest visual step left in the 3D host.
- **Meshes as assets.** A `Mesh` class (VAO/VBO/EBO owned properly) and an OBJ loader, so models stop being hand-typed arrays; the crate becomes a loaded model.
- **Sky & atmosphere.** A gradient skybox/clear and distance fog — the heightfield stops floating in void.
- **Stretch (honestly flagged):** a read-only 3D viewport panel inside the editor, rendering the scene through MyGL's renderer. Only if the earlier arcs land on schedule; cut without guilt otherwise.

## Arc 10 — Ship 2.0 (Lessons ~95–98)

- **Performance pass.** A profiling readout (frame-time graph in the editor, draw-call and triangle counters in MyGL), one measured optimization chosen by data, before/after numbers in the devlog.
- **The public question.** *(Decision point for Zac:)* the repo goes public as the portfolio piece, or stays private with a public showcase page. Either way: a release page with screenshots, both games, and the story.
- **2.0.0.** Both games repackaged on the 2.0 engine, changelog, tag — and the Phase 2 retrospective devlog: what the engine is now, what it still isn't, and what Phase 3 would be about.

## Decisions Zac owns (needed before the arc named)

1. **Before Arc 5:** the scripting language — Lua, hand-bound (recommended: the game industry's default embed, tiny, rookie-legible, and ownable the way this engine owns its GL loader) or an alternative he names.
2. **Before Arc 6:** the second game's name and fantasy (platformer mechanics are planned; identity is his).
3. **Before Arc 10:** public repo vs private + showcase.
4. **Any time in Arc 1:** his Phase 1 playtest findings *are* Arc 1's spec — the arc re-plans around them, not vice versa.

*Planned Nov 15, 2026, the day Phase 1 shipped; revised the same evening at Zac's direction — the "complete engine" north star (file system, per-game scripting, multiple windows) re-ordered Arcs 4–8 and grew the phase from nine arcs (~46 lessons) to ten (~55). Same author, same rules, same standard: if it isn't verified by playing it, it isn't done.*
