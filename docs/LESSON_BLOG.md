# The MyEngine Lesson Blog

_The story of building a game engine from scratch, one lesson at a time. Maintained by Dietrich from the session devlogs and build notes — Zac builds, answers the explain-it checks in chat, and Dietrich keeps the record. Per-lesson technical detail lives in `docs/devlog/`; the roadmap lives in `docs/ROADMAP.md`._

---

## Lesson 1 — The Loop

**Built:** an 800×600 SFML window, a game loop, and a clock. The loop prints its own frame rate once a second.

**The idea:** an engine is a loop that repeats forever — input, update, draw. The clock hands each frame its **dt** (how long the last frame took), because everything that moves will move by *speed × dt*, never by a fixed step per frame. Uncapped, the loop ran at roughly **8,400 FPS** with frame times around 0.12 ms — a number that becomes the baseline every later lesson is measured against.

## Lesson 2 — Movement

**Built:** a 50×50 green square that moves with WASD and the arrow keys, stops at the walls, and travels at the same speed diagonally as straight.

**The idea:** three small truths that everything later stands on. Movement input is *polled* ("is this key held right now?") because movement is continuous, while the event loop is for things that happen once. A diagonal direction (1,1) is √2 long, so it gets **normalised** — divided by its length — or diagonals would be 41% faster. And position is **clamped** to the window so the player can't leave the world.

## Lesson 3 — The First Internal Window

**Built:** a Dear ImGui debug panel living inside the engine window: live FPS and frame time, the player's position, a colour picker for the background, and a speed slider.

**The idea:** immediate-mode UI. There are no persistent widget objects — the panel is *re-declared from scratch every frame*, and the variables are the state: a slider bound to `speed` doesn't send events, it simply writes the variable. Read widgets (text) report; write widgets (sliders, colour editors) reach straight into the program's data. Also the first colour lesson: ImGui thinks in 0.0–1.0 floats, SFML in 0–255 bytes, and converting between them is a divide one way and a multiply the other — a direction Zac nailed down in his explain-it after one reversal.

## Lesson 4 — The Game View

**Built:** the game now renders into an `sf::RenderTexture` — an off-screen canvas — and the texture is displayed inside a "Game View" panel. The game and the window it lives in are officially different things.

**The idea:** two-stage drawing. Stage one draws the game into the texture; stage two draws the editor around it and pastes the texture in as an image. `gameView.display()` doesn't *show* anything — it finalises the texture; the panel's `Image` call is what shows it.

**What broke:** the lesson's suggested texture-flip fix didn't compile. Reading the actual ImGui-SFML source revealed the SFML overloads take no UV parameters at all — and that the dedicated `RenderTexture` overload flips the image internally. The correct call is one line: pass the RenderTexture itself. First entry in what became a standing rule: *version-sensitive API details get checked against the pinned library source, never recalled from memory.*

## Lesson 5 — Play, Pause, Step

**Built:** the editor gains time controls. A Play/Pause button, a Step button that advances exactly one frame, Space as a pause toggle, and a game-time clock that only runs while playing.

**The idea:** there are **two clocks**. The editor's clock always ticks — panels stay alive, FPS keeps counting. The game's clock only ticks when the host lets it. A button in immediate mode returns `true` only on the frame it was clicked, so `if (ImGui::Button(...))` *is* the click handler. And the lesson's exam question — why doesn't input pile up while paused and teleport the player on resume? — got its answer in Zac's words: *"the direction increment ONLY happens in the instances where 'time passes'. it doesn't pool input."* Paused frames measure their dt and throw it away; nothing accumulates, so nothing can teleport.

## Lesson 6 — Docking

**Built:** the panels become a real editor layout — they snap together, tile, share resizable borders, and the arrangement survives restarts. Docking lives on a separate branch of ImGui, so the lesson started with a CMake pin change to `v1.91.8-docking`.

**The idea:** the layout isn't saved by anyone's code. ImGui writes it to `imgui.ini` automatically — delete the file and the layout resets, which is the experiment that proves it. The dockspace itself is re-declared every frame, like everything else in immediate mode.

**What broke:** nearly everything that could. The fresh configure appeared to hang (stalled git clones inside the VS Code terminal — diagnosed via Task Manager showing four git processes at 0% CPU, and cleared by a `git ls-remote` probe proving the tag and his GitHub access were fine). A rerun failed on the half-finished build folder left by the killed attempt. And then the lesson's own `DockSpaceOverViewport` call was wrong for this version — it takes a dockspace ID *first*, viewport second — caught by IntelliSense and verified against the pinned header. The most realistic engine-development lesson of the first six, by a distance.

## Lesson 7 — The Resizable Game View

**Built:** the game view fills its panel at any size. Drag the layout or resize the window and the RenderTexture is recreated to match — never stretched, never letterboxed — and the playfield genuinely grows and shrinks with it.

**The idea:** the panel *measures* itself and stores the size; the renderer *applies* it next frame, before drawing. Two guards keep it stable: the image is displayed at exactly the measured size (an overflowing image would summon a scrollbar, which shrinks the region, which shrinks the texture — a flickering feedback loop), and the movement clamps floor their max at 0 because `std::clamp` with max < min is undefined behaviour. Zac's explain-it produced the lesson's best sentence: *"we could have a 1000000 x 1000000 screen and it would still be the same player size"* — the player is 50 **world units**, not 50 pixels; a bigger window shows more world, it doesn't inflate it.

**What broke:** one stale line — the `Image` call still hardcoded 800×600 from Lesson 4 while the texture resized around it, squashing the square into a tall rectangle and summoning the scrollbar the lesson had just warned about. One argument fixed it.

## Lesson 8 — The Split

**Built:** the game climbs out of `main.cpp` into its own class — `Game.hpp` (the promise) and `Game.cpp` (the machinery). `main.cpp` is now purely the host: window, editor, panels. On screen, nothing changed at all.

**The idea:** a refactor with an agenda. `Game` renders into any `sf::RenderTarget&` it's handed and knows nothing about ImGui, panels, windows, or textures — ignorance as an architectural asset. Lesson 5's two clocks became physical: `playing` stayed in the host (the *decision* about time), `time` moved into the game (the *state* of time). The editor's dials (`speed`, `clearColor`) are public data; the player shape is private behind a `playerPosition()` peephole.

**What broke:** twice, instructively. First the surgery left two wires unattached — the `game.update(...)` call never made it into the new main, and the panel kept reading a dead `gameTime` local — so the game rendered beautifully and never moved. Then the linker refused: all four `Game` methods unresolved, because `Game.cpp` hadn't been added to the CMake target *and* re-configured. Zac's takeaway question — "whenever I add a new cpp file, do I have to include it in CMakeLists?" — is now a standing rule he owns: yes, every time; an unlisted file doesn't exist.

## Lesson 9 — The Runtime

**Built:** `Game.cpp` becomes a library, `GameLib`, and a second executable appears: `MyRuntime` — the same game in a bare window with zero ImGui in it. One game, two hosts: the first true Editor/Runtime split.

**The idea:** every C++ program gets exactly one `main()`, so two programs means two executables — and a library is how they share the game without compiling it twice or repeating massive build lists (Zac's phrase, and the right one). The runtime calls `update` unconditionally: pause turned out to never belong to the game — it was editor policy, a host choosing not to call update. His explain-it went further unprompted: an *in-game* pause menu is a different animal entirely, belonging to whoever builds a game with the engine. Also settled this lesson: CMake reads the file line-by-line at *configure* time to build a dependency graph, and the build follows the graph, not the line order — and fetch order matters precisely when one library configures against another (ImGui-SFML needs SFML already fetched).

## Lesson 10 — The Procgen Foundation

**Built:** the engine's procedural bedrock. A `Random` class in the library: one master seed, and **named streams** — "terrain", "decor" — each its own generator derived from the seed plus its name. The game scatters 12 slate blocks and 20 gold dots from the streams; the editor gains a Procgen panel with seed entry, a random-seed button, and per-stream reshuffle buttons.

**The idea:** a generator isn't random — it's a machine unrolling a fixed sequence from a seed, so *same seed, same world, forever*, in the editor and the runtime alike. Named streams are what make that survive growth: with one global RNG, a single extra draw anywhere reshuffles every system downstream; with streams, reshuffling decor cannot move one block — the panel's buttons prove it live. The machinery is chosen for determinism: `mt19937` (its sequence is specified by the C++ standard), `seed_seq`, and a hand-written FNV-1a name hash, because `std::hash` is explicitly unspecified and determinism is never rented from unspecified behaviour.

Then Zac pushed the design himself: the scatter only fills the fixed 800×600 logical space, because the generate functions take no size — while `update` is handed the live world size every frame. His conclusion, derived in chat: a world is a function of *all* its inputs — `world = f(seed, parameters)` — so region sizes (a forest's bounds, a city limit) can absolutely be inputs, as long as they're explicit parts of the recipe stored with the seed. What kills determinism isn't parameters; it's *hidden* inputs like the size of an editor panel. Streams all the way down.

## Lesson 11 — Noise, and a Handover

**Built:** terrain. A `ValueNoise` class in the library hashes an infinite lattice of values from (seed, x, y) on demand, blends between them with smoothstep interpolation, and layers octaves — double frequency, half strength — into fBm. The game samples it into a 40×30 tile field coloured by height (deep water up to snow), drawn as **one batched VertexArray** under the blocks and dots, with a Noise scale slider in the Procgen panel. The editor and the runtime render the identical continent for seed 12345 — screenshots in `docs/shots/`, and the match between them *is* the verification.

**What broke, twice instructively.** IntelliSense on Zac's machine rejected `std::lerp` — a legal C++20 call parsed under a stale standard setting; the interpolation is now written out by hand, which suits an engine being built to be understood. And a code review caught a real Lesson 10 bug hiding in plain sight: `m_random` was declared *before* `m_seed` in `Game.hpp`, and since members initialise in declaration order, the whole procgen system was being seeded from an uninitialised variable. It had only ever *looked* deterministic. The fix is one moved line and a comment that will outlive both of us.

**The handover.** Mid-lesson, Zac changed the arrangement: *"keep creating this project without me. I'll git pull it when it's ready... show me screenshots as each new feature is built."* So from Lesson 11 onward Dietrich builds solo — on a Linux rig in his own VM (the AI PC, it turns out, has no compiler at all), pushing through it to GitHub, with a screenshot per feature in `docs/shots/`. Zac's own last code commit — his `derivedSeed`, identical to the lesson's — was merged in on the way past. The explain-it questions remain in each devlog, answered in the Concept sections, for whenever he wants to read the reasoning back.

## Lesson 12 — The Console

**Built:** the engine can talk now. A static `Log` class in GameLib keeps a capped buffer of levelled entries (info / warning / error), mirrors every line to stdout for the runtime's terminal, and the editor renders it as a Console panel — colour-coded, auto-scrolling, with a Clear button. The game logs its own life: creation, seed changes, and one summary line per generation instead of a flood.

The design point worth keeping: **restraint is a feature**. `setNoiseFrequency` logs nothing, on purpose, because the editor's slider fires it continuously during a drag — a console that floods is a console nobody reads. And the first warning the engine ever produced is a genuinely useful one: re-applying the seed that's already active warns you the world will regenerate identically, surfacing a silent no-op you'd otherwise never notice. The screenshot in `docs/shots/` shows it in amber, triggered by an automated click on Apply seed.

Two small scars, both recorded in the devlog: ImGui's built-in font can't draw an em-dash (it renders `?`), so engine text stays ASCII until a fuller font is loaded; and a redirected `std::cout` is fully buffered, which briefly made a healthy log file look empty. Neither cost more than a few minutes — the console itself is what caught the first one.

## Lesson 13 — Config & Math Utilities (Arc 2 Closes)

**Built:** the toolbox. `Math.hpp` holds the small operations everything reaches for — `lerp`, `remap`, `smoothstep01`, `normalize` — and was adopted the same day it was written: Noise's interpolation and the player's movement now run on it, and their private copies are deleted. `Config` reads a plain `engine.ini`; both hosts register defaults, load the file over them, and take their window size, seed, noise scale, and player speed from it. The verification screenshot is the fun one: a hand-written config with `seed = 777` and a chunkier noise scale, and the editor opens on a completely different continent with the panels already agreeing with the file.

The config's manners are the real lesson. A missing file isn't an error — the defaults get written out, so the first run creates the file you're meant to edit. A broken line isn't a crash — it's an amber console warning with a line number, and the rest of the file still applies (proven by deliberately breaking line 5). And the math half has the same moral in reverse: SFML ships a `normalized()` that would have saved ten lines, but reading its source first showed it asserts on a zero vector — and "no keys held" is the most common direction in any game. The engine keeps its own zero-guarded version *because* the library was checked. Arc 2's foundations — seeds, noise, logging, config, math — are done.

## Lesson 14 — Entities (Arc 3 Opens)

**Built:** the player lost its privileges. Until now it was a special shape living inside `Game` with its own movement code and its own draw call. Now there's an `Entity` — a plain data struct: name, position, size, colour, a flag for player control — and `Game` owns a list of them. The player is entity zero. Two orange crates joined it through the exact same `spawnEntity` door, and everything since has been loops: movement applies input to *whichever entity is flagged*, rendering derives a rectangle from each entity's data on the fly. Swap the flag to a crate and the crate drives — nothing else changes. That's the whole idea, and the runtime screenshot shows all three citizens on the continent, with the spawn lines in the log to prove they arrived by the same road.

The design restraint is deliberate and the devlog argues it openly: a struct of data, not a class hierarchy and not a full ECS, because the pressure that justifies components — entities needing *different subsets* of data — hasn't arrived yet. It will (crates don't need input; ghosts won't need collision), and the struct will split then, under real pressure instead of speculation. One honest limitation rides along: the crates aren't solid. The player ghosts straight through them, because entities are currently pure data plus rendering. Collision is the very next lesson, and it lands on a system that was built expecting it.

## Lesson 15 — AABB Collision

**Built:** solidity. Entities gained a `solid` flag, the toolbox gained an overlap test, and movement became axis-separated: apply X, push back to exact contact if anything overlaps; apply Y, same. The colliders are a *set*, not a type — solid entities and Lesson 10's terrain blocks feed the same resolution function through the same (position, size) shape, which is the entity design paying its first dividend. Diagonal-into-wall movement slides instead of sticking, because a blocked axis never cancels the free one.

The verification is the part worth telling: the editor was driven by automated key input — up for a beat, then left, straight at a crate. The screenshot shows the player resting flush against the crate's face, and the debug panel reads **Player: (240, 153)**. That 240 is the crate's right edge, exactly — the contact plane, computed. Without collision the same keys land the player at x≈95, inside the lake on the far side. The build also produced the arc's best error message so far: a missing `};` in `Entity.hpp` made the C++ standard library itself appear to explode, forty errors deep, none of them within a mile of the actual fault. The devlog records the moral — when the STL "breaks", look at the last header you touched.

## Lesson 16 — The Camera (Arc 3, Part 3)

**Built:** the world outgrew the screen. `Game` now owns a world size — 1600×1200 — and generation, movement, and clamping all read it; `update` doesn't take a size from anyone anymore. That closes the question Zac himself opened back in Lesson 10, when he spotted generation hardcoding 800×600 while movement followed the canvas: the canvas is now fully demoted to a window you look through, never a boundary. On top of that sits `Camera` — a centre and a visible height, nothing more. Width derives from the canvas aspect (distortion is impossible), zoom *is* the visible height changing, and follow is centre-on-target plus a clamp that keeps the view inside the world.

The screenshots carry the proof three ways. The runtime at spawn shows the familiar continent — same noise, same coordinates — except it now continues past every edge of the window. Drive into the corner and the framing doesn't move a pixel: the camera parks at the world's edge and the player walks off-centre to stand in the corner, which is the clamp working exactly as designed. In the editor, six mouse-wheel clicks over the Game View pull the zoom to 0.531 and nearly the whole 1600×1200 world fits on screen at once, dark bands of honest nothing beyond its edges. The runtime also quietly deleted its window-resize handler this lesson — when the view is re-derived every frame, there is nothing left for the handler to maintain. Arc 3 has one lesson left: sprites and animation, and the coloured rectangles retire.

---

_Blog continues from Lesson 17. Since the handover, Dietrich builds and keeps every record; Zac pulls when he's ready._
