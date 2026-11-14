# Devlog 005 — Play, Pause, Step (Editor Time vs Game Time)

## Goal

Give the editor control over the game's clock: Play/Pause buttons in the Debug panel (Space works too), a Step button that advances the game exactly one frame while paused, and a game-time readout that only advances while the game is playing.

## Concept

There are now **two clocks**. The editor's clock never stops: the window, the panels, and ImGui update every frame no matter what, or the tool would freeze solid. The game's clock only ticks when the game is playing. Pausing doesn't stop the loop — it stops the *simulation* from consuming dt. `gameTime` accumulates dt only while playing, so it freezes on pause while the FPS readout (editor time) keeps ticking. That's the visible proof the two clocks are separate.

Two supporting ideas:

- **An ImGui button is an event disguised as a return value.** `ImGui::Button(...)` returns `true` on exactly one frame — the frame the click lands — and `false` every other frame. No listeners, no callbacks; the `if` around the call *is* the click handler.
- **Loop order is data flow.** The Debug panel has to be declared *before* the movement code, because the simulation needs this frame's button results (`playing`, `stepClicked`). The panel also reads `player.getPosition()` directly now — it can no longer borrow the `pos` variable that the movement code computes later.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- Setup: `bool playing = true;` and `float gameTime = 0.f;`.
- Event loop: Space toggles `playing` (a discrete press = an event, per Lesson 2's distinction).
- Loop reordered: ImGui `Update` + the Debug panel declaration move *above* the movement code. Panel gains a Play/Pause toggle button, a Step button (`ImGui::SameLine`), a game-time readout, and reads position via `player.getPosition()`.
- Movement wrapped in `if (playing || stepClicked) { gameTime += dt; ... }` — the existing direction/normalise/clamp code unchanged inside.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — paused: player frozen, game time frozen, FPS still ticking, panels alive; Step with a movement key held: player inches one frame per click; unpause after a long pause: no teleport jump)_

## Explain-it check

Step 1:
create a boolean variable called playing and a float variable called gameTime. These will check if the game is running or is paused, as well as the elapsed game time

Step 2:
In the event polling loop's key press checking, add a check for the space bar that will effectively play or pause the game (playing vs !playing).

Step 3:
When we are updating the game debug menu, add a play/pause button that will pause the interaction in the game view. The step button will move by milliseconds to allow you to step through the current frame.
This will be useful for watching vfx happen frame by frame later on, but is useful for tracking the individual components of the game

Step 4:
Move player motion below the debug view and only trigger it when we have either clicked step button OR if the game is playing. otherwise the player is frozen and cannot be moved.

Addendum (the no-teleport question, answered in chat right after):
the teleport doesnt happen because the direction increment ONLY happens in the instances where "time passes". it doesn't pool input

## Next step

Lesson 6 — to be decided from here (docking/layout for the editor shell, or the first step from "a square" toward entities).
