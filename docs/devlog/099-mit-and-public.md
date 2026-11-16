# Devlog 099 — The Door Opens: MIT, and the Repo Goes Public

## The decision

Arc 10's gate was the author's alone — public repo or private
showcase — and on Nov 16, 2026 he made it in one tap: **go public,
MIT license.** The reasoning that fits this project: MyEngine is a
portfolio piece and a teaching document. Its value compounds when a
stranger can clone it, read the devlogs against the code, and build
a game with it. MIT is the license that asks nothing of that
stranger but attribution — the same license the author chose for his
pixel-art-tools repo in October, for the same stated reason: "I want
people to know that it's fully open source."

## What shipped

- `LICENSE` at the repo root: the standard MIT text, Copyright (c)
  2026 Zachary Shade.
- The README gains a License section naming what is *not* the
  author's to license: the bundled DejaVu font (its own license,
  shipped alongside) and the vendored Khronos headers (MIT, Khronos
  Group). An honest license page says where its edges are.
- The repository's visibility flipped from private to public
  *after* the license commit was on `main` — the repo never sat
  public without its license in place. Verified from outside:
  the GitHub API answers unauthenticated requests for the repo and
  detects the license as MIT.

## What going public does and doesn't change

The build, the harness, the devlogs, and the playtest gate are
unchanged — they were written from Lesson 1 as if strangers would
read them, because the roadmap always ended here. What changes is
the audience the paperwork speaks to, and one new standing rule
the phase inherits: anything committed now is published the moment
it is pushed, so the tree stays clean of secrets, tokens, and
other people's private information — audited before this flip
(the Arc 10 readiness audit) and by habit after it.
