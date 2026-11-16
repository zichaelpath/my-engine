# Roadmap — Phase 3: The Engine Feels Standard

*Sketch, set by Zac's direction on Nov 16, 2026. Phase 3 starts after
Arc 10 (Ship 2.0) closes Phase 2. Lesson numbers are estimates to be
fixed when the phase opens; the arcs and their order are the plan.*

## Why this phase exists

The 1.8.0 playtest gate put the engine in its author's hands on his
own machine, and his first three findings were not about rendering or
gameplay — they were about what it *feels like to open the thing*:

1. The ImGui windows should be in a **default layout** when the
   engine launches, like any other engine.
2. A project currently has to have the engine as its base — you work
   inside the engine's own folder.
3. You can't **add files** to a project from inside the editor.

Phase 2 made the engine *capable* (projects as folders, scripting,
campaigns, a 3D view). Phase 3 makes it *standard*: the furniture a
rookie expects to find, in the places they expect to find it. This is
the North Star's "any rookie game developer can download it and
create a video game" — measured at the front door, not the renderer.

## Where the three findings stand today (honest baseline)

- **Layout:** docking exists (`DockSpaceOverViewport`), but no layout
  is ever built in code. The arrangement comes from the repo's tracked
  `imgui.ini` — one person's mid-arrangement working layout, shipped
  to every clone. Delete it and ImGui cascades windows by default.
- **Projects:** a project *is* a folder (Lesson 60) and the editor can
  adopt one (Lesson 62), but the editor still launches assuming its
  working directory is the project — and the engine repo itself is
  the Gold Rush project, source and game content in one tree. There
  is no start screen, no create-project flow, no recent list.
- **Files:** the Project panel is a read-only tree with previews.
  Creating a folder happens exactly once, inside project creation,
  by the API — never from the panel. No rename, delete, duplicate,
  import, or create-anything.

## Arc 1 — First Launch Feels Like an Engine (~4 lessons)

- The default layout, built in code with the DockBuilder on first
  run: Game View centre, Hierarchy left, Inspector right, Project and
  Console tabbed along the bottom, tool panels (Procgen, Feel,
  Physics, Terrain, Levels, Engine Debug, 3D View) docked as a tab
  group, not a scatter.
- `imgui.ini` stops being project state: untracked, gitignored,
  per-user — your arrangement is yours, a fresh clone gets the
  default, and the repo stops shipping a stranger's window positions.
- A View menu: every panel toggleable, plus **Reset to default
  layout**. A layout version stamp, so a future engine update can
  re-default exactly once instead of fighting an old file forever.
- First-run behaviour with no saved state at all (delete-everything
  test): the engine opens arranged, every time, on both platforms.

## Arc 2 — The Project Is Not the Engine (~5 lessons)

- A **start screen** when no project is open: New Project, Open
  Project, and a recent-projects list (stored in a user-level config,
  not in any project).
- **Create Project from the editor:** name, location anywhere on
  disk, and a starter template (empty, or the Gold Rush-shaped
  starter) — the `createProjectFolder` API finally gets its UI.
- The editor launches with **no project adopted** and says so: a grey,
  honest empty state instead of silently adopting the folder it
  happens to be standing in. The engine repo stops being a game
  project; Gold Rush moves to `games/` beside Temple Run, a peer.
- Per-project editor state (last-open scene, the project's own
  layout) lives in the project folder, so two projects remember two
  desks.
- The runtime and packager already take the project as an argument —
  this arc makes the editor catch up with its own tools.

## Arc 3 — Files You Can Touch (~5 lessons)

- The Project panel grows a context menu and a Create button:
  **New Folder, New Scene, New Script, New Entity Types** — created
  in the selected folder, named inline, valid from birth (a new scene
  opens; a new script carries the engine's script skeleton).
- **Rename** (inline, F2), **duplicate**, and **delete** with a
  confirmation that names what references the file if anything does.
- **Import:** bring an image or sound into the project from anywhere
  on disk via the file browser — copied in, never linked out.
- Double-click does the obvious thing per kind: scene opens, script
  opens, image previews, sound plays. Drag a scene onto the Levels
  panel; drag a script onto an entity.
- The panel notices external change (a file added by Explorer or an
  IDE) and refreshes itself — the tree is a view of the disk, not a
  snapshot of it.

## Arc 4 — Standard Editing Furniture (~5 lessons)

- **Undo/redo** for scene edits — the single biggest "this feels like
  an engine" feature still missing. One history, Ctrl+Z / Ctrl+Y,
  covering entity create/delete, field edits, and terrain strokes.
- Multi-select in the Hierarchy; copy/paste/duplicate entities;
  Delete key deletes (with the selection, not a right-click hunt).
- The shortcut pass: Ctrl+S saves, F2 renames, and the editor shows
  an **unsaved-changes dot** — with a prompt before closing or
  switching projects over unsaved work.
- Selection survives play-test: what you had selected in edit mode is
  what you return to.

## Arc 5 — Ship 3.0 (~3 lessons)

- The sample project: a tiny game a rookie can open from the start
  screen and press Play on within a minute of downloading.
- "Your First Project" — the guide that walks the start screen, the
  default layout, and the Create menu, in the walkthrough style of
  `WINDOWS_BUILD.md`.
- Version 3.0.0: changelog, both sample games repackaged, clean-room
  verified from empty folders on the Linux rig and by Zac on Windows.

## Gates and standing rules

- Arc 10's decision (public repo vs showcase) still gates Ship 2.0;
  Phase 3 does not start until 2.0 ships or Zac re-sequences.
- Every lesson keeps the house rules: a testable scene or a change to
  one, harness green, paperwork in the same commit, and anything
  GUI-shaped gets verified by Zac's hands on Windows — the playtest
  gate is permanent now, not a phase.
