# Devlog 042 — One Point Zero: A Version, a Package, a Tag

## Goal

Give the engine a version that means something, rebuild the Gold Rush package as the 1.0.0 release (now carrying the project file), verify it clean-room, tag the release in git, and write the Windows build walkthrough — the one part of this arc that executes on Zac's machine, not this one.

## Concept

- **A version number is a build input, not a string in the code.** The single source of truth is the `project(MyEngine VERSION 1.0.0)` line in CMakeLists; `configure_file` bakes it into a generated `Version.hpp`, and all three hosts print it as their first log line ("MyEngine 1.0.0 - Runtime"). Bump one line, rebuild, and every binary agrees. The failure mode this prevents is the classic one: three hosts, three hand-typed version strings, two of them stale.
- **A release is a tag plus an artifact, and both got receipts.** The git tag `v1.0.0` pins the exact tree; the zip is the tree's game, staged by the same script as 0.1.0 — bumped to 1.0.0, now including `project.myengine` — and verified the Lesson 34 way: unzipped into an empty folder, run standalone, title screen byte-identical to the in-repo run, startup log reading the shipped project file.
- **The Windows walkthrough is written to be boring.** Prerequisites (VS2022 C++ workload as compiler provider, CMake, Git), configure/build/run commands, the run-from-repo-root rule, the new-`.cpp` re-configure rule from Lesson 8, and a DLL contingency step phrased as a check rather than a claim — the Linux build links SFML statically, and whether a given Windows configure produces DLLs is something the walkthrough tells Zac how to *see*, not something it guesses. The PowerShell packaging block mirrors the shell script file for file.

## Code built

- `CMakeLists.txt` — `project(... VERSION 1.0.0)`, `configure_file` for the generated header, GameLib include path for it.
- `src/Version.hpp.in` — the template: `engine::VERSION` plus major/minor/patch constants.
- All three hosts log the version banner first.
- `tools/package_game.sh` — VERSION 1.0.0.
- `CHANGELOG.md` — the 1.0.0 entry: the game, the editor, the engine, the GL host, in release-notes language.
- `docs/WINDOWS_BUILD.md` — the walkthrough above.

## What broke

Nothing. The generated header landed where the build expected it, the package script's new staging step worked first try, and the clean-room run matched the in-repo run pixel for pixel (same screenshot hash — the determinism habit paying off again).

## Verification

Build log clean; generated header contains 1.0.0; Runtime's first log line is the banner; the zip listing shows the binary, assets, `project.myengine`, and `VERSION.txt` (contents: 1.0.0); the clean-room screenshot is the title screen from the packaged binary in an empty directory. The tag exists locally and is pushed with this lesson.

## Explain-it check

_(Post-handover format: why is the version generated from CMake instead of typed into a header? What does the tag guarantee that the zip doesn't, and vice versa? Why does the Windows walkthrough phrase the DLL step as a check? — the answers live in the Concept section above.)_

## Next step

Lesson 43 closes the roadmap: the documentation pass (README as the portfolio front door — what this is, how to build it, where the design lives), a final all-hosts verification, and the honest "what 1.0 is and isn't" that every good release ends with.
