# Devlog 061 — The Place, Visible: The FileSystem Panel

## Goal

Lesson 60 made the project a place; this lesson makes the editor *show* it. A Project panel lists the whole tree under the root — folders, files, sizes — with refresh and selection. The listing logic lives in the Project module as a tested API (`listProjectFiles`); the panel is a view over it.

## Concept

- **The panel is a view, not a scanner.** `listProjectFiles` walks the root recursively, returns sorted project-relative entries, and is verified in the harness against a real temp project — the panel code itself only renders what it is given and remembers two things: the cached listing and the selection. When import (Lesson 63) and scene files (Lesson 64) arrive, they will refresh this same listing; nothing about the tree is panel-private.
- **Hidden means hidden.** Entries with a leading dot are skipped, and hidden directories are not descended into: `.gitkeep` and its kin are plumbing that keeps folders alive in git, not project content a rookie should have to wonder about. The harness plants a `.hidden` file and asserts its absence.
- **IDs are paths, labels are leaves.** Two files named `level1.scene` in different folders are ordinary in a real project; ImGui identifies widgets by their label text, so each row's ID is the full project-relative path (the `###` idiom) while the eye sees only the leaf name, indented by depth. A collision class designed out before it could happen.
- **The tree is honest about this repo.** Opened on MyEngine itself, the panel lists the entire repository — because the repository *is* the Gold Rush project place (root "."). That is not noise; it is the truth of how this project has always been laid out, and the standard subfolders sit in it where Lesson 60 put them.

## Code built

- `src/Project.hpp/.cpp` — `ProjectEntry` + `listProjectFiles` (recursive, sorted, hidden-skipping, sizes for files).
- `src/main.cpp` — the Project window: name + root header, Refresh, the indented tree, and the selected file's path and size.
- `tests/tests_main.cpp` — the listing check (suite now 40).

## Verification

- Harness: **40 passed, 0 failed** — directories and files listed with correct relative paths, the planted file's size read back exactly (7 bytes), the hidden file absent, a rootless project listing nothing.
- The editor itself: ImGui's saved layout records `[Window][Project] Pos=60,60 Size=395,714` from a live run — the largest panel in the editor, because the place it shows is real.
- Screenshot: `docs/shots/lesson-61-project-panel.png` — the panel over the editor, captured live.

## Explain-it check

_(Post-handover format: why does the listing live in the Project module instead of inside the panel's draw code? Why is each row's ImGui ID the full path while its label is just the leaf? Why are hidden files skipped rather than shown greyed? — the answers live in the Concept section above.)_

## Next step

Lesson 62: choosing files from outside the project — an in-editor file browser dialog (portable, no native-dialog dependency) that Open Project and the coming Import will both use.
