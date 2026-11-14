# Devlog 004 — The Game View (RenderTexture)

## Goal

Stop drawing the game straight onto the window. Draw it into an off-screen `sf::RenderTexture` instead, then show that texture inside its own ImGui panel — the game becomes a picture inside the editor, with panels around it. The app window grows to 1280×720; the game world stays 800×600.

## Concept

A RenderTexture is a **canvas that isn't a screen**. It has the same verbs as the window — `clear`, `draw`, `display` — but `display()` there means "finish drawing into the texture," not "show it." Nothing reaches the real screen until something draws the texture, and in an editor that something is a panel: `ImGui::Image` displays the texture like a photograph of the game.

This is the editor trick everything else builds on. Once the game is just an image inside a panel, panels can sit beside it, it can be resized, paused, or shown twice — the world no longer owns the window. It also moves the meaning of the coordinates: the movement clamps (0–750, 0–550) now describe the edges of the *game view*, not the window.

## Code built

_(filled at the start of the lesson; adjust if the build differs)_

- Window resized to 1280×720.
- `sf::RenderTexture gameView` (800×600) created once in setup, before the loop.
- Draw restructured into two stages: (1) clear/draw/display into `gameView`; (2) declare a "Game View" panel with `ImGui::Image(gameView, ...)` — pass the RenderTexture *itself*, not `getTexture()`: the library's RenderTexture overload flips the UVs internally (RenderTexture contents are stored upside-down), while the plain Texture overload does not. The SFML `Image` overloads take no UV arguments at all — after size come tint and border *colours* — so a hand-rolled UV-flip call doesn't compile; the raw-ImGui route would need the texture's native handle. Clear the real window to dark grey, `ImGui::SFML::Render`, `display`.
- Debug panel unchanged — its clear-colour picker now recolours the game view's background, not the whole window.

## What broke

_(yours to fill in)_

## Verification

_(yours to fill in — game visible inside its panel, player moves and clamps inside the view, speed slider and colour picker still work, panels draggable)_

## Explain-it check

Step 1:
Create a render texture below the player creation but before the loop with a 800x600 window

Step 2:
In the game view and above where the drawing of the whole engine lives, have the game view draw the background (clear), the player, and then call display to finalize the texture that will be displayed in step 3.

Step 3:
Call ImGui Begin and define the game view INSIDE the engine via ImGui::Image. SUPER IMPORTANT, don't call gameView.Texture() as the first parameterl it will flip the image.
Instead pass the entire gameView object as the first parameter, with the second parameter being the window area.
Call ImGui End to end the definition of that gameView for that frame.

Step 4:
For the ENGINE WINDOW, give it a clear color. Then have ImGui render the window object. Then call window.display() to display the window

## Next step

Lesson 5 — to be decided from here (panel layout/docking, or play/pause control of the game view).
