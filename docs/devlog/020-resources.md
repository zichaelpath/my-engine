# Devlog 020 — Resources (Textures Grow Up and Move to Disk)

## Goal

Textures stop being only code-generated. Introduce a `TextureCache` — the engine's first resource manager — load a real image file through it, and make failure graceful: a missing file becomes a visible placeholder and one log line, never a crash or a silent blank.

## Concept

- **A resource manager is a cache with a loading habit.** `TextureCache::get(path)` loads on first request and returns the stored texture forever after. Callers ask by path and never own, load, or free anything. The editor's debug panel now shows the cache size, because a cache you can't observe is a leak waiting for an alibi.
- **Two storage decisions, made on purpose.** The cache is a `std::map` because its node storage never invalidates references when new entries are inserted — callers hold texture references across later loads, and a `vector`-style container would eventually reallocate out from under them. And `get()` is `const` with a `mutable` map: loading on demand doesn't change the cache's *observable* state, only its contents. That pattern (logical constness) is how every real engine's resource system is shaped, and it's why `Game::render` — a const method — can fetch textures.
- **Failure is a texture, not an exception.** A failed load stamps the entry from a placeholder image — the magenta/black checkerboard that has meant "missing texture" since before this engine's author was born — and logs one error. The failure is cached too: a missing file is reported once, not once per frame. The game keeps running, and the missing asset *advertises itself* on screen, which is exactly what you want during development.
- **The crate emigrates.** The repo gains `assets/crate.tga` (drawn by `tools/make_assets.py`, a pure-Python TGA writer kept as the art's reproducible source: vertical planks, iron bands, rivets — visibly different from the sheet's crate) and the crate entity now renders from that file through the cache. The player still comes from the code-built sheet. Two texture sources, one render path — a normal transitional state for a growing engine. Asset paths resolve relative to the working directory: run from the repo root.

## Code built

- New `src/Resources.hpp/.cpp`: `TextureCache` (map-backed, placeholder image built in the constructor, load-once semantics with logging). Added to GameLib (with re-configure).
- New `assets/crate.tga` + `tools/make_assets.py`.
- `Game`: owns a `TextureCache`; the crate branch of the entity renderer draws from `assets/crate.tga` through it; `textureCacheSize()` exposed for the editor.
- Editor: "Textures cached: N" in the debug panel.

## What broke

Nothing — clean build, both verification paths first try. One log-reading subtlety worth recording: the no-assets run's stderr contains two "failed to load"-ish lines, but only one is the cache's — SFML prints its own complaint during the single load attempt. The cache logged its error exactly once, as designed; the second line is the library talking to itself.

## Verification

Built clean on the Linux rig (with re-configure for the new `.cpp`). Two runtime runs: (1) `docs/shots/lesson-20-assets.png` — with `assets/` present, the log carries "Texture loaded: assets/crate.tga" and both crates wear the new file-backed art (vertical planks, iron bands — visibly not the sheet's diagonal-brace crate), while the traveller still renders from the code-built sheet. (2) `docs/shots/lesson-20-placeholder.png` — the same binary in a folder without `assets/`: both crates become magenta/black checkerboards, the error is logged once, and the game runs on regardless. Graceful failure, demonstrated by deliberately breaking the environment — the only honest way to test a fallback.

## Explain-it check

_(Post-handover format: why a map and not a vector or unordered_map for the cache? Why is the failed load cached instead of retried next frame? What does the magenta checkerboard buy you that a blank space wouldn't? — the answers live in the Concept section above.)_

## Next step

Lesson 21 — tilemaps: the noise field becomes a data-driven tilemap (a grid of tile ids with a tileset), the last Arc 4 item — then Arc 5 grows the editor (hierarchy, inspector, selection).
