# Devlog 025 — Drag to Move (The First Gizmo, In Embryo)

## Goal

Grab the selected entity in the Game View with the mouse and drag it around the world; the Inspector's position fields follow along. Clicking was for choosing — this is for *placing*.

## Concept

- **A drag is a tiny state machine.** Three states wearing one boolean: idle, grabbed (button down on an entity), dragging (mouse moving while held). The transitions are the whole feature: press-on-entity starts it, release anywhere ends it, and the middle just answers "where is the mouse in world space now?" Every gizmo this editor will ever grow — scale handles, rotation, multi-drag — is this machine with extra states.
- **The grab offset is the difference between a tool and a toy.** On grab, the editor stores `entity position − mouse world position`. During the drag the entity sits at `mouse + offset`, so it stays exactly where your hand took it instead of snapping its corner (or centre) to the cursor. One subtraction, and dragging feels *right* — this is the detail every first drag implementation gets wrong and every user feels without naming.
- **Drags outlive the panel.** The continuation and the release are handled outside the hover check: pull the mouse past the Game View's edge mid-drag and the entity keeps tracking; release anywhere and the drop lands. Editors that cancel your drag because the cursor crossed a border teach users to move slowly and resentfully.
- **Log the drop, not the journey.** The console gets one line on release ("Moved Crate to (697, 448)") — per-frame logging would flood it 60 times a second for a single gesture. The same discipline as the noise slider in Lesson 12: continuous inputs report their *outcome*, not their samples.
- **The dragged entity is kinematic.** While held, it doesn't collide-resolve itself — if you drop a crate onto the player, the *player* gets resolved out next frame, because collision only ever moves the player. Fine for now, and stated plainly: editor placement is authoritative; the simulation adapts around it.

## Code built

Editor only (`src/main.cpp`): drag state (`dragging`, `dragOffset`) beside the selection; pick-on-press now also arms a drag with its grab offset; a continuation block after the Game View's hover handling moves the entity with the mouse in world space and logs the drop on release; scene loads cancel any drag along with the selection.

## What broke

The first drags moved *the Game View window* instead of the crate — by exactly the drag delta, which was the clue. ImGui's default lets a window be dragged from anywhere in its body when the press lands on no widget; the canvas image is not a widget, so the gesture belonged to the window frame, and since the panel chased the mouse 1:1, the computed world point never changed and the entity "correctly" stayed put. Single clicks in earlier lessons never exposed it (no motion, no window drag). The fix is one line of editor configuration — `ConfigWindowsMoveFromTitleBarOnly = true` — after which the same gesture drags the entity. The drag code itself never changed; the window manager was eating its input.

## Verification

Built clean on the Linux rig. After the fix, a scripted grab on the crate at the transform-computed point pulled it from (560, 380) to (662.7, 437.0) — a delta of (+102.7, +57.0) against the (+104.5, +58.0) the screen delta predicts through the view transform. `docs/shots/lesson-25-editor.png` shows the landing: the crate outlined at its new spot, the Inspector's position fields reading 662.662 / 437.034, the console's single "Moved Crate to (662, 437)" line, and an F5 save carrying the new coordinates in `scene.txt`. The Game View panel, pointedly, did not move.

## Explain-it check

_(Post-handover format: what goes wrong without the grab offset? Why are the drag's continuation and release handled outside the panel's hover check? Why log on release rather than during the drag? — the answers live in the Concept section above.)_

## Next step

Lesson 26 — spawn and delete: create new entities from the editor (a crate at the camera's centre) and delete the selected one. Deletion is where index-based selection gets honest scrutiny — removing an element shifts every index after it, and the editor's bookkeeping has to survive that.
