# Devlog 101 — The Default Layout: Phase 3 Opens at the Front Door

*Phase 3, Arc 1 (First Launch Feels Like an Engine), Lesson 1.*

## The finding it answers

The author's first playtest note, verbatim in spirit: the ImGui
windows should be in a default layout when the engine launches,
like any other engine. What a fresh clone actually got was the
repo's tracked `imgui.ini` — a mid-arrangement working layout
(the author's own, from an earlier session) shipped as cargo —
and, underneath it, no designed default at all: delete the file
and ImGui cascades windows wherever it pleases.

## What was built

- **`buildDefaultLayout()`** in the editor: a DockBuilder pass
  that runs on the first frame of a first run. Game View centre
  (Game View 2 and the 3D View tabbed behind it — views belong
  with views), Hierarchy left at 20% of the width, Inspector
  right at 24%, and the tool/output panels — Project (in front),
  Console, Engine Debug, Procgen, Feel, Physics, Terrain,
  Levels — tabbed along a bottom strip 28% of the height.
- **First run is detected honestly:** no `imgui.ini` beside the
  editor means no saved layout exists, so the default is built.
  From that moment the file takes over as it always has — the
  user's arrangement, saved automatically, surviving restarts.
  The default is a floor, not a landlord.
- **A way back:** Engine Debug gains a Layout section with
  **Reset to default layout** — the same builder, re-run on the
  next frame. One code path for first-run and reset, so the two
  can never drift apart.
- **`imgui.ini` stops being project state.** It was already in
  `.gitignore`; it is now also untracked (`git rm --cached`).
  A fresh clone has no layout file at all — which is precisely
  the condition the new first-run path is built for. (Existing
  clones: delete or `git checkout -- imgui.ini` before pulling;
  the next launch builds the default and starts a fresh,
  personal file.)

## Verification

On the Linux rig, from no `imgui.ini`: the editor's first run
writes a docking tree whose measurements are the design —
left node 255 of 1280 (20%), right node 307 (24%), centre node
marked `CentralNode=1`, bottom node 202 of 720 (28%) — and a
screenshot fragment shows Hierarchy and Game View docked side
by side in it. A second run loads the saved file untouched:
same DockSpace, same nodes, no rebuild. The Reset button's
click path is GUI interaction — it shares the verified builder
function, and its click joins the author's playtest gate like
every GUI path before it.

## Why this lesson is first

Phase 3 is the front-door phase, and the layout is the first
thing a stranger's hands touch. An engine that opens arranged
says the author finished something; an engine that opens in
someone else's half-dragged windows says the opposite — in the
first ten seconds, before a single feature gets a chance.
