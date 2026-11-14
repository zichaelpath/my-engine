# Devlog 006 — Docking (A Real Editor Layout)

## Goal

Make the panels dock: drag the Engine Debug and Game View panels by their title bars and snap them into a fixed layout inside the engine window, the way Godot's and Unity's panels tile together — and have the layout remembered between runs.

## Concept

Docking isn't in the Dear ImGui build used up to now. It lives on ImGui's separate **docking branch** — same version (1.91.8), same library, an extra feature set maintained as its own line of code. Lesson 6 starts by re-pinning the download to the `v1.91.8-docking` tag.

With docking enabled, one call per frame — `ImGui::DockSpaceOverViewport(...)` — lays an invisible docking grid over the whole window. Dragging a panel by its title bar then shows drop targets, and the panel snaps in, tiling with its neighbours instead of floating over them. Panels can also be resized by their shared edges once docked.

Nobody writes save code for the layout: **ImGui persists it automatically in `imgui.ini`** next to the executable (delete the file and the layout resets — that's the proof of where it lives). And note what's *not* enabled: tearing panels out into separate OS windows (ImGui's multi-viewport feature) — ImGui-SFML doesn't support that out of the box, and an editor doesn't need it yet.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- CMake: Dear ImGui pin changed from `v1.91.8` to `v1.91.8-docking` (Build folder deleted, fresh configure).
- After `ImGui::SFML::Init(window)`: `ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;`
- Every frame, after `ImGui::SFML::Update` and before the first `Begin`: `ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());`
- No panel code changes — docking is a property of the space, not of the panels.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — panels snap to edges and tile, shared edges resize them, layout survives a restart, deleting imgui.ini resets it)_

## Explain-it check

_(yours to fill in)_

## Next step

Lesson 7 — the game view resizes with its panel: recreate the RenderTexture at the panel's content size (carefully — a texture sized by the panel that displays it can feed back on itself).
