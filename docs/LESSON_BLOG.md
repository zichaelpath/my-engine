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

## Lesson 17 — Sprites & Animation (Arc 3 Closes)

**Built:** the rectangles retired. The engine now paints its own sprite sheet in code at startup — no image files anywhere — a 160×32 atlas holding a little hooded traveller in four walk poses and one very earnest wooden crate. Entities name a sprite, rendering cuts the right 32×32 window out of the sheet and scales it to the entity's size, and the movement system drives the animation: the walk clock advances only while moving (a frame every 0.12 seconds), stopping settles back to idle, and horizontal input flips the sprite's facing. The world screenshot says most of it — the green square is a *character* now, standing on the shore of the lake, and the orange squares became crates with planks and a diagonal brace. The close-up pair, grabbed a third of a second apart mid-walk, shows the stride and passing poses alternating: animation proven the only honest way, with two frames that differ.

That closes Arc 3: the world has things in it, the things are solid, the camera roams a world bigger than the screen, and the things have faces. Arc 4 turns to data — saving and loading scenes, managing resources, tilemaps — which is where Arc 2's determinism starts paying compound interest: a saved world is mostly a seed plus a handful of entity records, because everything else can be regenerated exactly.

## Lesson 18 — Scene Save/Load (Arc 4 Opens)

**Built:** persistence, and the file is almost comically small — nine lines for an entire 1600×1200 world. That's Arc 2's determinism collecting its debt: the terrain is a function of (seed, noise scale), so the scene stores the *recipe* and regenerates the meal on load. What's left is one line per entity — name, position, size, colour, flags, sprite — in a text format you can open in Notepad and edit by hand, versioned from day one because the cheapest moment to add a version line is before a second version exists. Loading is all-or-nothing: parse into temporaries, refuse a wrong version, warn-and-skip a corrupt line, and only touch the live world once the file has proven itself whole.

Both hosts got F5/F9 quicksave/quickload, and the verification was a proper round-trip: walk the traveller to (542, 455), save, walk away, load — and the debug panel reads (542, 455) again while the console narrates the save, the regeneration, and the load in order. The devlog states the one honest limit in writing: reshuffle history isn't saved, because the file stores the recipe rather than the RNG streams' diary. That's a v1 boundary, named rather than hidden — exactly where it belongs.

## Lesson 19 — The Hitbox Fix (A Detour Zac Ordered)

**Built:** a bug fix that became a lesson, because Zac played the game and reported that the movement hitbox "feels really weird" — and he was right. The diagnosis is the interesting part: the collision system was pixel-perfect (the player rested at exactly x=510 against the crate, the mathematically correct contact plane), and it *still* felt wrong, because the box being collided was the sprite's full 50×50 square while the traveller inside it is only about 34 px wide with transparent margins. Invisible padding stopped him early, invisible corners snagged on scenery his body cleared. The fix separates the two rectangles every game eventually separates: the sprite is what you see, the **hitbox** is what you collide with — in the traveller's case a 28×22 "feet box" at the bottom of the sprite, so his feet tell the truth and his hood can lean over things, which even reads as depth.

The before/after crops are the whole argument in two pictures: a strip of water between hand and crate before, hand pressed to the wood after — and an F5 save file recording the new resting place, x=521, as the numeric receipt. The editor also gained a Show hitboxes overlay, because a feel problem you can see is one you can tune. Resources, originally this lesson, slide to Lesson 20 without complaint: dogfood feedback outranks roadmap order, and the roadmap exists to serve the game, not the other way round.

## Lesson 20 — Resources

**Built:** the engine's first resource manager, and with it the crate emigrated to disk. `TextureCache` loads a texture the first time a path is requested and serves the stored copy forever after; the crate now renders from `assets/crate.tga` — a real file, drawn by a small Python TGA-writer kept in `tools/` as the art's reproducible source — while the traveller still comes from the code-built sheet. Two texture sources, one render path. The design details are the quiet kind that matter later: a `std::map` because its references survive insertions, a `const` getter with a mutable cache because that's how a const render method can fetch textures, and failure handled as a *texture* rather than a crash — a missing file stamps the entry with a magenta checkerboard and logs one error.

The fallback got the only honest test there is: run the same binary in a folder with no `assets/` and watch. Both crates turned into magenta checkerboards, the error appeared exactly once, and the game didn't blink. A missing asset that advertises itself on screen is worth ten that fail silently — during development you *want* your mistakes loud and pink. One lesson remains in Arc 4: tilemaps, where the noise field becomes real data — a grid of tile ids — instead of a baked picture.

## Lesson 21 — Tilemaps (Arc 4 Closes)

**Built:** the terrain stopped being a picture. Generation now classifies the noise into tile ids first — deep water, water, sand, grass, forest, rock, stored in a grid the engine can query — and builds the familiar vertex picture *from* those ids, so the world looks pixel-identical while being something entirely new underneath: data. The first customer arrived immediately. Movement now asks the map what tile is under the player's feet and scales speed by the answer — wading at half pace, sand dragging, rock worse — and the editor gained a Tile readout so you can watch the ground truth change as you walk.

The verification has the best story of the arc: the control run on "grass" came back suspiciously slow, twice, and the brand-new Tile readout is what exonerated the code — the chosen meadow was actually a pond shore, and the run had waded across a lake. Measured properly from inside the western lake, one second of walking covers 108 px against dry land's 200. The instrumentation caught the experiment lying, which is exactly what instrumentation is for. Arc 4 is complete — scenes, resources, hitboxes, tilemaps — and Arc 5 turns the editor from a viewer into a tool: selection first, then hierarchy and inspector.

## Lesson 22 — Selection (Arc 5 Opens)

**Built:** the editor learned to point. Click any entity in the Game View and it's selected — yellow outline in the world, name and position in the debug panel, a line in the console; click empty ground and the selection clears. The interesting part is where the feature *doesn't* live: selection is pure editor state. `Game` wasn't touched this lesson — the editor does the picking (panel pixel → texture pixel → world position → topmost-first search through the sprite boxes) and even draws the highlight itself, into the same texture right after the game renders. The runtime will never know selection exists, and that's the architecture working as intended.

Verification had a one-pixel comedy: the first scripted click missed the crate by a hair of world space because the test assumed a slightly wrong view width — the pick honestly reported "empty ground", the aim was recomputed from the camera readout, and the second click landed centre-crate. Next, the Hierarchy panel gives the same selection a second door: a list of every entity, clickable — proving selection is a model, not a mouse trick.

## Lesson 23 — The Hierarchy Panel

**Built:** a Hierarchy panel listing every entity by name — Player, Crate, Crate — where clicking a row selects it. The lesson is deliberately tiny, because its point is architectural: selection turned out to be a *model*, not a mouse trick. One shared index, two independent writers (the Game View picking from last lesson and now this list), and every reader — the yellow outline, the debug readout — lit up for list clicks without a single line changing. Panels are views onto shared state, never owners of private copies; this is the shape the whole editor will follow.

The list also tells a truth the world view hides: the two crates have the *same name*, sitting in adjacent rows like a small accusation. That's what hierarchies are for — exposing scene hygiene problems — and it sets up the next lesson perfectly: the Inspector, where the selected entity's name, position, size, colour and flags become editable, and "Crate / Crate" can finally become "Crate by the lake / Crate at spawn".

## Lesson 24 — The Inspector

**Built:** the editor's first *writer*. Select an entity and the Inspector shows its real properties — name, position, size, colour, solid flag — every one of them editable, writing straight into the live game while it runs. Drag a crate's position and it moves in the world; rename it and the Hierarchy row updates in the same frame ("Crate / Crate" finally became "Crate / Lake Crate"); untick Solid and the player can walk through it. `Game` grew exactly one thing for this: a mutable entities accessor for tools, which the runtime never calls.

The verification renamed the lakeside crate and then saved the scene — the file itself contains "Lake Crate", proof the edit lives in the game's data and not just in a panel. Two honest limits are on the record: colour edits are real but invisible on sprite-bearing entities (tinting is a future line), and the sprite field is read-only because swapping a sprite must also swap its hitbox. Next: drag-to-move — grab the selected entity in the Game View and pull it around, the embryo of every gizmo to come.

## Lesson 25 — Drag to Move

**Built:** grab any entity in the Game View and pull it around the world. Under the hood it's a three-state machine (idle, grabbed, dragging) plus one crucial subtraction — the grab offset, entity position minus mouse position, so the crate stays where your hand took it instead of snapping to the cursor. The drop logs a single line; the Inspector's position fields follow along live; drags survive leaving the panel mid-pull.

The first attempt dragged *the window* instead of the crate — ImGui's default lets a press on a widget-less area move the window itself, and since the panel chased the mouse perfectly, the world point under it never changed and the entity stayed put with complete correctness. One configuration line (windows move from title bars only) gave the gesture back to the canvas, and the retest moved the crate to within a pixel of the predicted landing spot. Next: spawn and delete — where index-based selection meets the problem of a list that can shrink.

## Lesson 26 — Spawn & Delete

**Built:** the editor can now create and destroy. "Spawn crate" drops a new crate at the camera's centre — through the same `spawnEntity` the game itself uses, hitbox and all — and it arrives selected, ready to drag. Delete (button, or the Delete key when no text field has focus) removes the selected entity through a new `Game::removeEntity` — which flatly refuses the player, with an amber warning in the console, because every system in the game assumes a player exists.

Deletion is where index-based identity earns its scrutiny: erasing an element shifts every later index, so the editor clears its selection and the game repairs its player index in the same operation. All of it verified in one session — spawn (entity 4, selected, at the camera's centre), refusal (player delete attempted and declined), delete (count 4 → 3, selection cleared), and an F5 save showing exactly the surviving three. Next, the safety net every editor eventually needs: undo/redo.

## Lesson 27 — Undo/Redo

**Built:** Ctrl+Z and Ctrl+Y over every editor action — drags, spawns, deletes, and Inspector edits. The implementation is deliberately the simple kind: before each committed action, the editor snapshots the *whole entity list*; undo swaps the current list onto the redo stack and restores the snapshot. At thirty small entities, uniformity beats the command pattern's per-action inverse logic — and the devlog says plainly where that trade flips. Snapshots land at commit points only (a drag's grab, a widget's activation frame), so one gesture is one undo step, and restoring goes through a `Game` function that re-derives the player from its flag, so a restored list can never point the game at the wrong entity.

Verified as one continuous session against the saved file: drag, undo, redo, spawn, undo — the final `scene.txt` holds exactly the redone drag position with the spawn nowhere in it, and the Hierarchy's "1 back / 1 fwd" readout matches a hand-trace of the stacks. One lesson remains in Arc 5: play-in-editor with restore, so experimenting costs nothing at all.

## Lesson 28 — Play-Test With Restore (Arc 5 Closes)

**Built:** a Play test button that snapshots the scene, and a Stop button that restores it exactly — entities and game clock alike. Walk the player to the far corner, press Stop, and he's back at spawn as if the trip never happened. It looks like undo but answers a different question: undo is a history of *authoring actions*, while the test snapshot is a save point for an *experiment* — separate stacks, no cross-talk, and the panel says the contract out loud while you test ("changes are discarded on Stop"). Scene loads cancel test mode, because a snapshot of a scene that no longer exists would be a lie.

That closes Arc 5. In six lessons the editor gained every verb it needs — select, list, inspect, drag, spawn, delete, undo, experiment — and `Game` was touched exactly twice along the way, which is the shared-state architecture from Lesson 22 paying off. Arc 6 gives the engine ears: audio, with the resource cache's family growing a sound branch, and a world that finally makes noise — starting with the splash when you wade in.

## Lesson 29 — Audio Foundations: The World Makes Noise

**Built:** sound, in the engine's established resource shape. A `SoundBufferCache` mirrors the texture cache contract for contract — load once, cache forever, and a placeholder on failure (here a synthesized blip, so a missing file is audible instead of silent). Playback runs through an eight-voice pool, because a footstep never waits for the last footstep to finish. The first two sounds are synthesized from math by a tools script, like the crate art: footsteps on a 0.27-second cadence with alternating pitch, and a splash that fires on the *edge* between land and water — standing in the pond is silent, stepping in is not. Volume is a host setting (config key + editor slider); the game code never touches it.

SFML 3's audio API broke two of my from-memory assumptions (sounds are born with their buffers; sample loading wants a channel map) — both caught by the compiler and fixed against the pinned headers. One honest caveat: the build VM has no speakers, so playback is verified by logs, counters, and the splash line, with the actual *hearing* deferred to Zac's machine.

## Lesson 30 — Music: The Stream

**Built:** background music, streamed rather than loaded — a three-minute song as a `sf::SoundBuffer` would cost tens of megabytes of memory, while `sf::Music` decodes from disk in chunks and costs kilobytes. The track itself is synthesized by the tools script: four pad chords crossfaded with wrap-around envelopes so the loop point is seamless by construction, plus sparse plucks that all decay before the seam. Both hosts start it at boot behind a `music` config key, and the editor shows its state with Play/Stop controls. Music is presentation, not simulation — it doesn't pause with the game or rewind with a play-test.

The verification carries an honest asterisk, recorded in full in the devlog: this build VM has no audio device at all (and SFML 3 turns out to use miniaudio, not OpenAL), so the stream reports Playing and the controls provably work, but the playback offset can't advance without something consuming samples. The hearing test, like the sound effects', happens on Zac's machine.

## Lesson 31 — Bump and Click (Arc 6 Closes)

**Built:** the last two sounds. The bump needs no new collision plumbing: the resolver already computes the truth every frame — the player *asked* for so many pixels and *got* fewer — so a displacement deficit on either axis means contact, with crates and terrain blocks alike, and a short cooldown keeps a held push from machine-gunning the knock. The editor's selection tick borrows the game's sound pool, a small coupling named in the devlog with its graduation condition (a third consumer earns the pool its own Audio service). With that, Arc 6 is done: every sound in the engine — steps, splash, bump, click, music, even the placeholder blip — is synthesized from math by one tools script.

Next is the payoff arc: a real game on this engine. The decor generator has been scattering gold dots since Lesson 10 — time to make them collectible, add a win state and a timer, and ship it in the Runtime.

## Lesson 32 — The Gold Rush: A Real Game

**Built:** the payoff. The gold dots the decor generator has scattered since Lesson 10 are collectible now — walk over one and it's yours, with a rising-pitch pickup blip. Collect all 40 and the world is cleared: the run timer freezes (your time is your score), a win arpeggio plays, and the Runtime shows a banner offering R for a fresh, randomly-seeded world. The rules live in `Game` — the editor's panel shows the same Gold and Run time counters as the Runtime's new bundled-font HUD — because a game rule that only exists in one host isn't an engine feature, it's a demo trick.

Verification was the fun kind: at full-world zoom I detected every dot in the screenshot programmatically and drag-teleported the player onto each with the editor's own drag tool. The counter climbed to 40/40 CLEARED, the console logged the clear at 134.9 s, and the timer provably froze while the game clock ran on. Two dots survived the first pass by a few pixels — collection measures from the *feet*, not the sprite centre — which is exactly the kind of thing you only learn by finishing a world.

## Lesson 33 — The Frame: Title, Pause, Win

**Built:** the difference between a demo and a game is the frame around it, so the Runtime got one: a proper state machine with four screens — Title, Playing, Paused, Won — in a single enum. Enter starts the run, P (or Esc) pauses, and winning promotes you to a win screen showing your final time, with R dealing a fresh world. The simulation only advances in Playing, which buys a subtle correctness for free: the speedrun clock *cannot* start before the player does, because the frozen title backdrop never calls update at all. `Game` didn't change by one line — it already knew the rules; the Runtime just learned how to present a session.

The Won screen earned its verification the hard way: a throwaway parallel build tree collapsed (two CMake configures corrupting each other — my mess, documented in the devlog), so the win was proven by temporarily shrinking the world to 3 dots in the real tree, sweeping the map quadrant by quadrant with programmatic dot detection, and keyboard-driving onto each one until the banner froze the clock at 1:40. Then the tweak was reverted — the real game still has all 40.

## Lesson 34 — Gold Rush v0.1.0: The Package

**Built:** Arc 7 closes with the thing it was always pointing at — a game somebody else could play. `tools/package_game.sh` builds the Runtime, renames it `GoldRush`, stages only what the game loads (editor sounds and source stay home), adds a player README and a default config, and zips it as **GoldRush-0.1.0-linux.zip, 1.9 MB**. Because SFML links statically here, the binary carries its engine with it — no DLL hunts, no launcher scripts. The proof was a clean-room run: unzipped into an empty directory, launched cold, title screen up with every asset resolving from the package alone. One honest limit: this zip is the Linux build from my rig; a Windows package needs the same steps in Zac's Windows environment, and that's Arc 9 work.

Thirty-four lessons ago this was an empty window. It's now an engine with an editor, a sound-synthesizing audio system, a deterministic procgen world — and a shipped game. Arc 8 goes where the roadmap always promised: raw OpenGL, and the third dimension.

## Lesson 35 — Hello, Triangle

**Built:** Arc 8 opens with the smallest honest 3D program: a third host, `MyGL`, an OpenGL 3.3 core context inside an SFML window, and one triangle whose three corner colours the GPU blends across its face. The interesting part is what *isn't* there — no GLAD. A core context only links OpenGL 1.1 directly; everything modern has to be requested from the driver by name at runtime, so the engine grew its own loader: 19 function pointers, one per function it actually uses, each miss logged by name. Zac's rule for this project is *understand everything*, and a generated loader is exactly the kind of magic this engine refuses. SFML, meanwhile, is demoted to window-and-events duty — `sf::Window`, never `RenderWindow` — so no SFML renderer state fights the GL pipeline. The 2D game never noticed a thing.

Next: the triangle grows up — an indexed cube, depth testing, and the model/view/projection matrix chain.

## Lesson 36 — The Cube: Depth, Indices, and the MVP Chain

**Built:** the triangle grows up. The cube is 8 corners shared by 36 indices (the colours flow *across* the edges in the screenshot because the faces literally share vertices), the depth test keeps a per-pixel distance image so the back faces lose to the front ones, and the spin comes from the classic chain — model × view × projection — computed by a new header-only `Mat4` written for the engine: column-major, right-handed, -Z forward, no library. The loader grew by exactly two lines (the uniform functions the MVP travels through), keeping its census honest.

Next: a flyable camera — lookAt, WASD, and a field of cubes to move through — then the merger: the 2D tile world, extruded into 3D.

## Lesson 37 — Flying: lookAt and a Field of Cubes

**Built:** a camera you can fly. `Mat4` gained a `Vec3` and `lookAt` — which is really a small geometry lesson: forward from where you're looking, side from a cross product with world-up, true-up from another cross, and the view matrix as that basis turned inside out. WASD flies along the look direction (pitch down and W genuinely descends), Q/E handle altitude, the arrows look. The test scene is 25 spinning cubes off a single VAO — one mesh, 25 model matrices — and the proof is a pair of screenshots bracketing a flight plus the exit log reading the final position back: (0, 0.68, −2.29), 4.8 units from where it started, exactly on the path the inputs describe.

Next is the merger this arc has been promising: the Gold Rush tile world, extruded into a 3D heightfield you can fly over.

## Lesson 38 — The Merger: The Tile World, Extruded

**Built:** the two halves of the engine meet. MyGL now constructs the same `Game` Gold Rush plays, reads its 80×60 tile grid, and extrudes it: water low in its basin, sand at the shore, grass, forest, and rock standing highest — flat tops plus skirt walls on every step, 18,604 triangles in a single draw. The palette is one table now (`Game::tileColor`, hoisted out of the 2D builder), so the 3D world *cannot* drift from the 2D map. Flying the low pass over the lake terraces is the first moment the engine feels like one thing instead of two projects.

The lesson also produced my favourite verification so far: a pixel-diff against an old screenshot suggested the world had changed, so instead of arguing with a picture I compiled a probe against Lesson 32's code from a git worktree and compared full-grid checksums — identical, cell for cell. The screenshot had been framed through a leftover config; the world never moved. Instruments over eyeballs, again.

## Lesson 39 — Sunlight: Normals and a Directional Light

**Built:** real light. Every face of the heightfield now carries its exact normal (the mesh is axis-aligned, so the builder *knows* them — tops point up, skirts point along their axis), and the fragment shader runs Lambert's law against a western sun: one dot product, an ambient floor so shadow sides stay readable, and Lesson 38's hand-darkened skirt trick deleted — fakes don't get layered over, they get replaced. The low screenshot is the proof: individual blocks in the lake show a lit face and an ambient face, and the shading direction agrees across the whole map, which only a directional light does. The loader census grew by one (`glUniform3f`, the sun's channel).

One honest footnote the devlog records: the normals ride unrotated because the terrain's model is the identity — the day a rotated model carries normals, they'll need the inverse-transpose. Noted, not dodged.

## Lesson 40 — Texture: The Crate Comes Home

**Built:** Arc 8 closes. The crate texture — the same `crate.tga` the 2D game has used since Lesson 20 — is uploaded as a real GL texture, a 24-vertex indexed cube wears it via UVs, and the crates aren't placed by hand: they're read out of the Game's entity list, converted from 2D pixels to terrain space, and stood on the height of the tile beneath them. Move a crate in the 2D world and its 3D self moves. Two shader programs now share one sun.

The lesson's best moment was a compiler refusal: the four texture functions I tried to add to the loader don't have loadable types — because texture objects are OpenGL **1.1**, from 1997. Textures are older than shaders. They left the loader and are called directly, like `glClear`. The close-up screenshot — plank grain readable, right way up, lit like the terrain around it — is the whole arc in one frame.

## Lesson 41 — The Project File: An Identity Card for a Game

**Built:** Arc 9 opens with the piece everything else in the arc stands on. `project.myengine` — four lines: format, name, seed, start scene — now says what game this is. The Runtime takes its window title, its title-screen shout, its world seed, and its start scene from that file; the Editor reads it for identity; the package script ships it with the game. The duties split cleanly: the project file travels with the game and defines it, `engine.ini` stays on the player's machine with their window and volume, and the project wins conflicts on its own turf.

The verification is my favourite kind — a four-line test card in a scratch folder (`name=Star Salvage`, `seed=777`) turned the *same binary* into a visibly different game, title screen and all. And the loader is strict where it matters: a file from a future format version fails loudly and whole, never half-loads into a subtly wrong game.

## Lesson 42 — One Point Zero: A Version, a Package, a Tag

**Built:** the release machinery. The engine's version now lives in exactly one place — the `project()` line in CMakeLists — and flows through a generated header into all three hosts, each of which announces "MyEngine 1.0.0" as its first log line. Gold Rush was repackaged as 1.0.0 (the project file rides along now), clean-room verified from an empty folder with a title screen byte-identical to the in-repo run, and the tree is tagged `v1.0.0`. `CHANGELOG.md` tells the release story, and `docs/WINDOWS_BUILD.md` is the walkthrough for the one machine I can't build on — written to be boring, including the DLL step phrased as a check rather than a guess.

## Lesson 43 — The Front Door: Documentation Pass and Final Verification

**Built:** no code — the last lesson is the telling. The README was rewritten from a 42-lesson status log into a front door: what the engine is, both hero screenshots, all four targets at final scope, build and controls for every host, the three file formats, and a closing section on what 1.0 is *not* — no scripting, no physics beyond AABB, no 3D in the editor — because a portfolio that names its limits reads as more credible, not less. Final verification ran the whole tree fresh: clean build, every host up and observed.

The roadmap is complete. Forty-three lessons from an empty `main()` to a tagged 1.0.0: an engine with a deterministic procedural heart, an editor that restores your scene exactly, a shipped game, and a 3D world grown from the 2D one's data. Zac — it's yours. Pull `main`, build it on your machine, and play Gold Rush. The next arc is yours to name.

---

_The blog ends at Lesson 43 — the roadmap complete, MyEngine 1.0.0 shipped. The devlogs remain the full record._

---

# Phase 2

## Lesson 44 — First Contact: The Left-Facing Ghost

Phase 2's first lesson wasn't built; it was *reported*. Zac pulled 1.0.0, built it on Windows, played it — and sent back a screenshot: the player's red hitbox floating a full body-width left of the character. Pixel-measuring his screenshot and then a headless render probe pinned it down: the hitbox was never wrong. Since Lesson 17, the sprite's left-facing flip branch had set an SFML origin on top of the position shift that mirroring already needs, so a left-facing player *drew* one body-width right of where he *collided*. Facing right, picture and body agreed — which is how it hid for 27 lessons. One-line fix, verified by the probe: hood pixels moved from x 438..462 (outside the entity box entirely) to 388..411 (centred over the feet box). His session caught a second real bug on the way: the Windows build died on `KHR/khrplatform.h`, a header Linux had been lending us silently — now vendored from Khronos. Arc 1 is open, and its thesis is already proven: the engine's first real player found in one afternoon what its test screenshots couldn't.

## Lesson 45 — The Probes Grow Up: A Test Harness

Phase 1 verified itself with throwaway probes — a checksum program compiled for one afternoon, a pixel-measuring script for one bug. Lesson 45 makes them permanent: `MyTests`, a fifth target that checks the engine's contracts in one command. Same seed, same world (with a hard fingerprint of the 1.0 world, 936912399, so generation can never change silently); scenes round-trip; project files load, refuse the future, and shrug at absence; config survives junk lines; and the Lesson 44 flip bug is now a regression check that renders the game and measures the sprite against its hitbox. Ten checks, all green. The harness's first run taught its own lesson by aborting: a `Game` can't even be *constructed* without a display, because the sprite sheet is a GL texture — so the suite detects a missing display and skips honestly instead of dying, and CI will run it under a virtual display on purpose. It also earned a structural fix in passing: GameLib finally exports its own headers properly, something only an outside consumer could have exposed.

## Lesson 46 — CI: A Second Machine That Never Forgets

The harness from Lesson 45 now runs itself. A GitHub Actions workflow builds the entire engine from a clean checkout on Linux *and* Windows for every push, and runs MyTests on both — the Windows job being, pointedly, the exact build that broke on Zac's machine in Lesson 44, automated before he ever pulls again. The Linux test step doesn't trust the harness's exit code (a suite that skips wholesale still exits 0, by design): it greps the log for "0 failed" *and* a real passed count, so green has to mean the checks ran. First run: both jobs green — Linux in 2m43s, Windows in 5m54s — and the job logs, read back rather than assumed, show the full line on both: 10 passed, 0 failed, 0 skipped, render check included. Phase 1's defects escaped because they needed a player and a different OS; the engine now gets the different OS on every single commit.

## Lesson 49 — Easing & Tweens: The Player Pops In

Arc 2 (Game Feel) opens with the primitive under half of all juice: values moving over time along curves. `Easing.hpp` holds seven of them — the quad/cubic staples, a `outBack` that overshoots, a `outBounce` that lands — each landing on exactly 1, because a curve that ends at 0.999 makes every animation it ever drives end slightly wrong. `FloatTween` carries one float along one curve; Gold Rush's player is its first customer, popping in from nothing at every run start, feet anchored, overshooting a touch before settling. The hitbox ignores the whole performance — Lesson 44's rule: the picture may play, the body stays honest. Verification is this arc's template: the harness checks the math (endpoints, overshoot, clamping, settle-at-exactly-1), and a render probe reads the scale through the engine's own getter — 0.549 at a twentieth of a second in, precisely what the formula predicts. Fifteen checks, all green.

## Lesson 50 — Particles: The Splash Gets Its Picture

A particle pool joins GameLib, and with it the engine's second feel primitive: emitters as pure data. The gold burst and the water splash are the *same code* — one is a full-circle spray under light gravity, the other a narrow cone under heavy gravity; effects differ in numbers, not in systems. Particles live in a flat vector with swap-with-last deaths and a hard cap, draw as a single triangle array fading with age, and — the discipline that matters most — roll their randomness from the dedicated "fx" stream, so confetti can never perturb the deterministic world it decorates. The harness proves that claim rather than asserting it: the 1.0 grid checksum still passes in the same run that fires bursts. Eighteen checks green, and the splash that has sounded right since Lesson 29 finally looks right too.

## Lesson 51 — Camera Feel: Trauma, Hit-Stop, and a Camera That Glides

The camera gets a nervous system. Events no longer shake anything directly — they add *trauma*, and the visible rattle scales with trauma squared, which is why a wall bump whispers (+0.15) and the winning dot shouts (+0.35) through the same code. The offset is a pure function of game time — two mismatched sines — so it is smooth, irregular, and testable to the decimal. Hit-stop freezes the world for 120 ms at the win; its first draft quietly ate the remainder of the frame it expired in, the harness caught the run clock losing time, and the fix hands the leftover dt back to the world. And the follow camera finally glides: exponential approach, plus a lookahead led by the player's real post-collision velocity, capped so a sprint never outruns it. Twenty-one checks green; a probe measured the rattle at (−8.4, +7.5) and the screenshot displaces by exactly that.

## Lesson 52 — Gold Rush, Juiced

The feel layer stops demoing and starts *being the game*. Pickups now feed a streak — grab gold within 1.2 seconds of the last grab and the burst grows (10 + 2 per streak step) — so a flowing run looks different from a plodding one, and the window forgetting is half the design: a reward that never resets stops being information. Every pickup also opens a shockwave ring, expanding on an ease-out curve; stepping out of water drips, the mirror of Lesson 50's splash. The win became a sequence with beats: freeze and rattle first, then — only once the hit-stop releases the world, a ordering guaranteed by where the fountain clock sits in `update` — a fountain over the player, closed by a wide slow ring. That staging forced a cross-host fix: the Runtime had to keep updating the world behind the Won screen, with the player politely ignoring input and taking his bow. Twenty-four checks green, including a drip counted to exactly six particles through real tiles and real updates.

## Lesson 53 — Feel Is Data: The Editor Panel, and Arc 2 Closes

Arc 2 ends by handing its keys over. Every constant the feel lessons tuned into the code — the pop's 0.35 seconds, the shake's 14 units, the streak's 1.2-second memory — now lives in one struct, `Game::Feel`, with the shipped numbers as its defaults, and the editor has a Feel panel that edits it live: a master switch that silences the whole layer, sliders for every dial, readouts for trauma/streak/particle counts, and Fire buttons that trigger the game's own event hooks so you tune a burst by watching it, not imagining it. The harness proves the three readers agree: defaults reproduce Lessons 49–52 exactly (every earlier check passed unchanged), the master switch off reproduces the pre-Arc-2 game, and retuned numbers change behaviour in exactly the way their names promise. Twenty-six checks green. Game Feel is done: the engine can now animate, burst, rattle, freeze, glide — and be told not to, from inside its own editor.

## Lesson 54 — A Second Way to Move: Gravity & Grounded

Arc 3 opens by teaching entities a second way to move. The movement model is per-entity data — `TopDown` or `Platformer` — so one world can mix them, and the platformer integrates a real velocity (gravity at 1800 units/s², terminal fall 900) into the *same* axis-separated collision resolver the top-down game has trusted since Lesson 15: one collision truth for both games, no fork. Grounded is not a state you enter but a question re-answered every frame — gravity always asks for more downward travel, and when the world allows less than was asked, you're standing. The tests caught the lesson's best wrinkle before it could become folklore: a player spawned *inside* a terrain block gets resolved politely on top of it, because the resolver assumes you arrived from your movement side — so the fall test finds a clear column empirically, and the warning is now written down: spawn points must not overlap solids. Twenty-eight checks green; the probe strip lands the player on a crate at exactly the predicted height, to the decimal.
