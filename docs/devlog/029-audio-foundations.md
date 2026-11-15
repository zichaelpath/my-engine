# Devlog 029 — Audio Foundations: The World Makes Noise (Arc 6 Opens)

## Goal

Sound in the engine: a sound resource cache beside the texture cache, a playback pool for one-shots, and the first two real sounds — footsteps while walking, and a splash when the player wades into water.

## Concept

- **Sounds are resources, so they get the texture treatment.** `SoundBufferCache` mirrors `TextureCache` exactly: load on first request, cache forever, `std::map` for reference stability, logical constness, and a placeholder on failure — here a short synthesized blip, so a missing file still makes *a* sound and the error is logged once. Two caches with the same contract is a pattern, and the pattern is the lesson: resources are a solved shape in this engine now.
- **Buffers are data; players are voices.** A `sf::SoundBuffer` is the sample data (shared, cached, cheap to reference). A `sf::Sound` is a *voice* that plays a buffer. One-shot sounds need a **pool of voices** — eight slots, round-robin, prefer a free one, steal the oldest when all are busy — because a footstep must never wait for the previous footstep to finish. Music will be a different beast (streamed, one voice) next lesson.
- **The assets are synthesized, like the placeholder.** `tools/make_sounds.py` writes `step.wav` (a low thud with a dropping pitch) and `splash.wav` (low-passed noise with a swell-and-fall envelope) from pure math with a fixed seed — the sounds are deterministic and reproducible from the repo, same philosophy as `make_assets.py`.
- **Gameplay triggers are edges and cadences, not states.** Footsteps run on a cadence clock (a step every 0.27 s while moving, pitch alternating 0.94/1.06 so the loop doesn't sound mechanical, timer pre-loaded so a new walk's first step lands quickly). The splash is an *edge detector*: it fires on the transition land→water in the tile stream, once per entry — standing in water is silent, which is correct.
- **Volume is a host setting.** `sf::Listener`'s global volume comes from the `volume` config key (default 80) in both hosts; the editor adds a live slider. The game code never touches it — how loud the world is belongs to the listener, not the world.
- **The rig note:** SFML's audio module needs system libraries the graphics build didn't (Vorbis/Ogg/FLAC/OpenAL — installed on the Linux rig). This VM has no speakers, so verification ran on OpenAL's null driver: playback is exercised for real, just silently, and the witnesses are the cache logs, the splash line, and the played counter — the same instruments-over-eyeballs rule as always.

## Code built

- `Resources`: `SoundBufferCache` (placeholder blip synthesized in its constructor).
- `Game`: the 8-slot playback pool + `playSound(path, pitch)`; footstep cadence and splash edge in `update`; played/cache counters for the editor.
- `tools/make_sounds.py` + generated `assets/step.wav`, `assets/splash.wav`.
- Hosts: `volume` config; editor volume slider + sound readouts; SFML::Audio linked into GameLib; the Linux rig's build now compiles SFML with audio on.

## What broke

SFML 3's audio API is *not* the SFML 2 API from memory: `sf::Sound` has no default constructor (it is born with its buffer — the pool now creates slots lazily on first use), and `loadFromSamples` requires a channel map argument (`{sf::SoundChannel::Mono}`). Both were caught by the compiler and fixed against the pinned 3.0.2 headers — the standing rule (verify version-sensitive APIs against the shipped headers) paying rent again.

## Verification

Built clean on the Linux rig (SFML recompiled with its audio module). Two driven walks in the editor, witnessed by the instruments: an east-then-west walk played **10 footsteps** at the 0.27 s cadence with `assets/step.wav` loaded exactly once into the cache; a second walk — east onto dry land, then west back into the pond — ended with the player standing in Deep water, the console showing "Sound loaded: assets/splash.wav" and **"Splash - the player wades in"**, and the counter at 13 played / 2 cached. The first attempt's route never produced a land→water edge (the player *starts* in the shallows and only exited), so no splash fired — the edge detector was right and the test was wrong; the corrected route is the one described. Playback ran on OpenAL's null driver (the VM is deaf), so the sounds are verified as *triggered, loaded, and played through the pool* — Zac's machine will be the first to actually hear them.

## Explain-it check

_(Post-handover format: why do buffers and players exist as separate things, and why a pool? Why is the splash an edge detector rather than a state check? Why does a missing sound file produce a blip instead of silence? — the answers live in the Concept section above.)_

## Next step

Lesson 30 — music: a streamed track (SFML `sf::Music` doesn't load whole songs into memory), synthesized as a calm loop by the tools, with play/pause in the editor and the loop running under both hosts.
