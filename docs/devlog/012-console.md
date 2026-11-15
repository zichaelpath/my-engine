# Devlog 012 — The Console (The Engine Learns to Speak)

## Goal

Give the engine a voice: a logging system in GameLib that any system can write to, rendered by the editor as a Console panel with level colours, auto-scroll, and a Clear button — and mirrored to stdout so the runtime gets the same lines in its terminal.

## Concept

`std::cout` is not a logging system: it has no levels, no memory, and nowhere to live inside an editor. A real log is three separate decisions:

1. **Capture** — a `Log` class with static `info` / `warning` / `error` calls and a stored buffer of entries (level + message), so lines outlive the moment they were printed. The buffer is capped at 500 entries, oldest dropped first — an engine that runs for hours cannot grow its log forever.
2. **Distribution** — the buffer is the single source of truth, and *hosts* decide how to present it. The editor draws the Console panel; every line is also mirrored to stdout, which is all the runtime has (and all it needs). Same lines, two surfaces, zero duplication in the systems that produce them.
3. **Restraint** — logging is a design decision per call site, not a reflex. Generation logs one summary line per operation (a seed change produces four readable lines, not four hundred). `setNoiseFrequency` deliberately logs *nothing*: the editor slider fires it continuously during a drag, and a console that floods is a console nobody reads.

Levels carry meaning by colour in the panel: grey for information, amber for warnings, red for errors. The first genuine warning in the engine is already wired: re-applying the seed that's already active warns that the world will regenerate identically — the kind of silent no-op a console exists to surface.

## Code built

- New `src/Log.hpp` / `src/Log.cpp` (in GameLib): `LogLevel`, `LogEntry`, and the static `Log` class (500-entry capped buffer, stdout mirror with level tags).
- `Game`: logs on creation ("Game created with seed …"), on seed set (info, or the re-apply warning), and one summary line per generation (field with its noise scale, terrain block count, decor dot count).
- Editor: a **Console** panel — Clear button, Auto-scroll checkbox, colour-coded entries in a bordered scrolling child; scrolls to the newest line when the entry count changes.
- `CMakeLists.txt`: `src/Log.cpp` added to `GameLib`.

## What broke

- **The em-dash incident**: the re-apply warning originally read "Seed 12345 re-applied — the world…" and the console rendered the `—` as `?`. ImGui's built-in font only covers basic Latin, so any character outside it shows as a placeholder. Standing rule from this: engine log strings (and, until a fuller font is loaded, all editor text) stay ASCII.
- **The empty log file that wasn't**: the editor's redirected stdout looked empty after a run — `std::cout` is fully buffered when writing to a file, and killing the process discarded the buffer. The lines existed the whole time (the console proved it); the stdout mirror is for interactive terminals, where line buffering makes it behave. Not a code bug — a reminder that a buffered stream is not a transcript.

## Verification

Built clean on the Linux rig. Editor screenshot (`docs/shots/lesson-12-editor.png`) shows the Console panel under the Game View: the four startup lines (field with noise scale, terrain count, decor count, game created), then — after an automated click on **Apply seed** with the seed unchanged — the amber warning "Seed 12345 re-applied: the world regenerates identically", followed by the three regeneration lines. An earlier run clicked **Reshuffle terrain** instead and showed its line appended live (and the blocks visibly moved), so capture, colour, live append, and auto-scroll are all exercised. The runtime prints the same lines to its terminal via the stdout mirror.

## Explain-it check

_(Post-handover format: the questions stay on the record — why is a stored buffer better than printing as you go? Why does the log live in GameLib instead of the editor? Why does `setNoiseFrequency` log nothing? — and the answers live in the Concept section above.)_

## Next step

Lesson 13 — config & math utilities: the small shared toolbox (a config the hosts can load, and the vector/math helpers everything later will reach for), closing Arc 2.
