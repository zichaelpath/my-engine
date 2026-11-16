# Devlog 102 — The View Menu and the Layout Stamp: Arc 1 Complete

*Phase 3, Arc 1 (First Launch Feels Like an Engine), Lesson 2.*

## What was built

- **A main menu bar.** The editor has never had one. It carries a
  single **View** menu for now — every panel as a checked item,
  grouped the way the desk is grouped (views, then the working
  panels, then the tool panels), with **Reset to default
  layout** at the bottom beside its Engine Debug twin. The menu
  is submitted before the dockspace each frame, and the pinned
  ImGui's `DockSpaceOverViewport` fills the viewport's *work
  area* — so the menu takes its 19 pixels off the top and the
  desk starts below it, no overlap, no hand-computed offsets.
- **A visibility flag per panel.** Eleven panels gained a
  `show*` bool and a guard around their block; Game View 2 and
  the 3D View promote their existing Engine Debug checkboxes
  into the same menu. Hiding a panel skips its whole block —
  its cost and its input go with it — and every flag defaults
  to shown: hiding is a desk-clearing choice, never a default.
- **The layout version stamp.** `kDefaultLayoutVersion` (1)
  is written to `engine.ini` as `layoutVersion` whenever the
  default layout is built. Startup now builds the default when
  there is no saved layout **or** the stamp is older than the
  code's version — so a future redesign of the default reaches
  existing users exactly once, then leaves their arrangements
  alone. The stamp rides the Config system (Lesson 84's
  set/save pattern), not a new file.

## Verification

Three runs on the rig, all read from the files the editor
itself writes:

1. **Fresh start** (no `imgui.ini`, no `engine.ini`): the
   default layout is built and `layoutVersion = 1` appears in
   the new `engine.ini`. The docking tree now reads
   `Pos=0,19 Size=1280,701` — the menu bar's 19 pixels,
   reserved and respected, in the editor's own numbers.
2. **A user's arrangement, stamped current:** a window's
   DockId was hand-moved to another node in `imgui.ini` with
   the stamp at 1; after a full run the move **persisted** —
   no rebuild, the user's desk is theirs.
3. **The same arrangement, stamp reset to 0:** the next run
   rebuilt the default (the moved window back in its designed
   node) and re-stamped `layoutVersion = 1` — the
   exactly-once re-default, working.

A screenshot fragment catches the View menu sitting over the
default layout's Hierarchy and Game View. The menu's clicks
themselves are GUI paths — they join the author's playtest
gate, beside the Reset button they mirror.

## Arc 1, closed

First launch now feels like an engine: it opens arranged
(Lesson 101), its layout file is the user's own, every panel
can be shown or hidden from a menu bar that did not exist two
lessons ago, and the default can evolve without stranding
anyone. Next: Arc 2 — the start screen, and projects that are
not the engine.
