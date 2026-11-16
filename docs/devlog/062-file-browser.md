# Devlog 062 — Choosing Things: The File Browser

## Goal

Give the editor a way to *choose* files and folders: a modal file browser, built in-editor (portable — no native-dialog dependency that would behave differently, or go missing, per platform), plus its data function `listDirectory`. Its first consumer ships in the same lesson: the Project panel's **Open project folder…** button. Its second consumer, Import, arrives next lesson and plugs into the file mode built here.

## Concept

- **One directory at a time, in the order everyone expects.** `listDirectory` returns a single folder's entries — directories first, then files, each sorted by name, hidden entries skipped. It is the browser's entire data model, it is small, and it is verified in the harness against a real folder (including the missing-folder case, which lists nothing rather than erroring).
- **Opening a project adopts it whole.** Choosing a folder and confirming loads its project card, and the editor takes the project on completely: the panel shows the new place, the world regenerates from the project's seed (`newRun`), and the project's start scene loads when it names one that exists — the same adoption the runtime performs at launch. Anything less (panel switches, world stays) would make "which project am I editing?" a question with two answers.
- **The browser is a modal with purposes, not a tangle of dialogs.** One popup, a purpose flag: 1 picks a folder (files shown greyed, for orientation), 2 picks a file (files selectable, a Choose button that reports through `dialogResult`/`dialogFinished`). New choosers later — scene save, script pick — are new purposes on the same browser, not new browsers.
- **Verified where it can be, named where it can't.** The listing order and edge cases are harness checks. The modal's click path — navigate, confirm, cancel — is GUI interaction, exercised by hand in the playtest, in the same honest category as the audio: the machinery underneath is proven, the clicking is Zac's.

## Code built

- `src/Project.hpp/.cpp` — `DirEntry` + `listDirectory`.
- `src/main.cpp` — the browser modal + dialog state; the Project panel's Open button; project adoption on confirm (card + seed + start scene + listing refresh). The panel's listing state moved beside the dialog state so the dialog can refresh it.
- `tests/tests_main.cpp` — the ordering check (suite now 41).

## Verification

- Harness: **41 passed, 0 failed** — a planted folder lists exactly `[assets/, scenes/, scripts/, project.myengine]` in browser order, hidden entries absent, a nonexistent path listing nothing.
- The editor runs with the dialog machinery in its frame loop (live-run smoke test); the modal itself opens on the Project panel's button.
- Screenshot: `docs/shots/lesson-62-file-browser.png` — the browser's data order over a real fixture folder.

## Explain-it check

_(Post-handover format: why build the browser in-editor instead of calling the operating system's dialog? Why does opening a project regenerate the world instead of only switching the panel? Why one modal with purposes instead of a separate dialog per chooser? — the answers live in the Concept section above.)_

## Next step

Lesson 63: asset import — the browser's file mode gains its consumer: pick a file anywhere, it is copied into the project's `assets/`, collision-safely renamed when needed, and the panel shows it.
