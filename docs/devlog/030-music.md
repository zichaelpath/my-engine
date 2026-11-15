# Devlog 030 — Music: The Stream (Arc 6 Continues)

## Goal

Background music: a calm synthesized loop, streamed from disk by `sf::Music`, started by both hosts, controllable from the editor.

## Concept

- **Music is not a big sound effect.** A `sf::SoundBuffer` loads a whole file into memory decoded — right for a 0.09 s footstep, wrong for a three-minute track (tens of megabytes). `sf::Music` is a *stream*: it decodes the file in chunks on its own thread as it plays, so a song costs kilobytes of memory regardless of length. Same speaker, different plumbing — choosing between them is a memory question, not a taste question.
- **The track is synthesized to loop.** `tools/make_sounds.py` grows a `music()` function: four pad chords (Am7 – Fmaj7 – Cmaj7 – G6, four seconds each) built from sine partials, crossfaded with *wrap-around* envelopes so the last chord blends into the first — the loop point is seamless by construction, not by editing luck. Sparse pentatonic plucks decorate it, all decaying before the seam. Sixteen seconds at half sample rate (22050 Hz — pads don't need the top octave) keeps the file at ~700 KB.
- **Music is presentation, not simulation.** It does not pause when the game pauses, does not rewind with a play-test, and is not part of the scene. It belongs to the *session*, which is why the hosts start it (behind a `music` config key, default on) and the editor merely offers a stop/play button. A missing track file logs one error and the game continues in silence — garnish is never a dependency.
- **The state lives in `Game` anyway.** One `sf::Music`, one track path, `startMusic`/`stopMusic`/`musicPlaying`/`musicOffset` — because both hosts need it and the editor needs to *show* it. The offset readout is the verification instrument on a deaf machine: a stream that reports Playing with an advancing offset is a stream that works.

## Code built

- `Game`: the `sf::Music` member and its small API (open → loop → play; failure logs once and returns false).
- `tools/make_sounds.py`: the `music()` generator; `assets/music.wav` committed.
- Hosts: `music` config key; both start the track at boot. Editor: Music status line (state + offset) and Play/Stop music buttons in the debug panel.

## What broke

Two rig discoveries, both recorded honestly. First: SFML 3's audio backend is **miniaudio, not OpenAL** — the Lesson 29 sessions' `ALSOFT_DRIVERS` setting was a no-op (harmless, but the record should be straight). This VM has no audio device at all (no `/dev/snd`), so miniaudio finds nothing to play to. Second: my first Stop-music clicks "failed" for the dumber reason — I aimed ~35 px high, at the readout text instead of the button, and spent three attempts suspecting the code before measuring the panel properly. The instruments rule cuts both ways: it also acquits the code.

## Verification

Built clean. In the editor: the boot log shows "Music started: assets/music.wav" (a failed open would log an error and return false instead), the panel reads "Music: playing", and the full control cycle is in the console — Stop button → "Music stopped" → Play button → "Music started: assets/music.wav". What could **not** be verified on this rig: actual playback progression. With no audio device, the stream reports Playing but its offset stays at 0.0 s — nothing consumes samples, so the offset and the 16 s loop wrap are unobservable here. On any machine with a sound device (including Zac's) the same code path advances and loops; that last step is asserted from SFML's documented behaviour, not from a local measurement, and the devlog says so.

## Explain-it check

_(Post-handover format: why does music stream instead of loading like a sound effect? How is the loop seamless — what would a naive 16-second render do at the wrap? Why doesn't the music pause with the game? — the answers live in the Concept section above.)_

## Next step

Arc 6 closes with a small one — Lesson 31: UI sounds in the editor (click feedback through the same pool) and a bump when the player walks into a crate, then Arc 7 begins: a real game on this engine.
