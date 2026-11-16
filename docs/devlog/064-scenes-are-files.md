# Devlog 064 — Scenes Are Files

## Goal

Retire the hardcoded `scene.txt`. The editor now tracks a **current scene** — a project-relative path inside `scenes/` — and everything scene-shaped flows through it: Save writes there, F9 reloads from there, **Save as…** and **Load…** go through the file browser, and double-clicking a scene in the Project panel loads it and makes it current. With no project open, the Phase 1 behaviour (plain `scene.txt`) is preserved untouched.

## Concept

- **The post-load ritual had three copies; now it has one.** Loading a scene in the old editor meant re-syncing the procgen fields, discarding the selection, clearing undo history (it indexes into an entity list that no longer exists), and killing any play-test snapshot — and that ritual was written out, by hand, at every load site. `loadSceneIntoEditor` defines it once; F9, the buttons, the browser, and the panel's double-click all call it. Duplicated rituals drift; a named one can't.
- **A first save adopts a home.** Saving an untitled scene inside a project creates `scenes/` if needed and adopts `scenes/untitled.scene` as the current scene — the file has a place and a name from its first save, and the panel refreshes to show it. Save As pre-fills the current name, offers existing scenes' names on click (standard dialog manners), and appends `.scene` when the typed name has no extension.
- **Current means inside the project.** Browser results arrive as absolute paths; the editor relativizes them against the project root, and a scene loaded from *outside* the project loads fine but leaves the current scene empty — it isn't the project's scene, and pretending otherwise would make the next Save write into a place the file didn't come from. Opening a different project likewise clears the current scene.
- **An honest build note.** The first build of this lesson failed: my F9 edit left a doubled closing brace, and a stale binary from the previous lesson briefly made a smoke test look alive. The harness (43/43, including the new scene-in-project round-trip) was unaffected — it builds different targets — but the editor verification was re-run against the fresh binary before this devlog was written. Check the timestamp, not just the pulse.

## Code built

- `src/main.cpp` — `currentScene` editor state; `loadSceneIntoEditor` and `saveCurrentScene` lambdas; F5/F9 and the Scene section rewired (Current readout, Save / Save as… / Reload / Load…); browser purposes 3 (save, with a name field) and 4 (load); the results consumer relativizing paths; Project-panel double-click-to-load for `.scene` files (and the legacy `scene.txt`); opening a project clears the current scene.
- `tests/tests_main.cpp` — the composed check: a scene saved through `projectPath` into a created project's `scenes/` loads back whole after the world is regenerated away (suite now 43).

## Verification

- Harness: **43 passed, 0 failed**.
- The editor, rebuilt fresh, runs with the new Scene section and browser purposes in its frame loop; the modal click-paths join the playtest-verified category (Lesson 62's note applies).
- Screenshot: `docs/shots/lesson-64-scene-files.png` — the editor's Scene section with its Current readout and the four actions.

## Explain-it check

_(Post-handover format: why was the post-load ritual worth extracting into one lambda? Why does a scene loaded from outside the project leave the current scene empty? Why does a first save adopt `scenes/untitled.scene` instead of asking for a name immediately? — the answers live in the Concept section above.)_

## Next step

Lesson 65 closes Arc 4: project-relative resources — the Project panel previews an imported image, loaded by resolving its project-relative path against the root, the same resolution everything else now uses.
