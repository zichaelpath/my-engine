#!/usr/bin/env python3
"""Generate the engine's starter sound effects as WAV files.

Same philosophy as make_assets.py: the sounds are synthesized from
math, so their "source" is reproducible and lives in the repo. No
recordings, no downloaded samples.

  assets/step.wav   - a soft footstep: a short low thud with a
                      breath of noise, ~0.09 s
  assets/splash.wav - wading in: filtered noise that swells and
                      falls, ~0.45 s

Run from the repo root:  python3 tools/make_sounds.py
"""
import math
import os
import random
import struct
import wave

RATE = 44100


def write_wav(path, samples):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        frames = b"".join(
            struct.pack("<h", max(-32767, min(32767, int(s * 32767))))
            for s in samples
        )
        w.writeframes(frames)
    print("wrote", path)


def step():
    n = int(RATE * 0.09)
    out = []
    phase = 0.0
    for i in range(n):
        t = i / RATE
        env = math.exp(-t * 55.0)  # fast decay thud
        freq = 140.0 - 500.0 * t  # pitch drops as the "foot" lands
        if freq < 60.0:
            freq = 60.0
        phase += 2.0 * math.pi * freq / RATE
        tone = math.sin(phase)
        noise = random.uniform(-1.0, 1.0) * math.exp(-t * 90.0)
        out.append((tone * 0.8 + noise * 0.35) * env)
    return out


def splash():
    n = int(RATE * 0.45)
    out = []
    smooth = 0.0
    for i in range(n):
        t = i / RATE
        # Swell in, then decay — the shape of water being displaced.
        env = (1.0 - math.exp(-t * 18.0)) * math.exp(-t * 6.0)
        # One-pole lowpass over white noise: rushing, not hissing.
        smooth += 0.18 * (random.uniform(-1.0, 1.0) - smooth)
        # A faint droplet chirp on top, sliding down.
        chirp = math.sin(2.0 * math.pi * (900.0 - 900.0 * t) * t) * math.exp(-t * 9.0)
        out.append((smooth * 1.6 + chirp * 0.25) * env * 0.9)
    return out


def bump():
    """A short woody knock for walking into something solid."""
    n = int(RATE * 0.07)
    out = []
    phase = 0.0
    for i in range(n):
        t = i / RATE
        env = math.exp(-t * 70.0)
        freq = 200.0 - 1400.0 * t
        if freq < 90.0:
            freq = 90.0
        phase += 2.0 * math.pi * freq / RATE
        click = random.uniform(-1.0, 1.0) * math.exp(-t * 300.0)
        out.append((math.sin(phase) * 0.85 + click * 0.3) * env)
    return out


def click():
    """A tiny UI tick for editor selections."""
    n = int(RATE * 0.03)
    out = []
    for i in range(n):
        t = i / RATE
        env = math.exp(-t * 160.0)
        out.append(math.sin(2.0 * math.pi * 2100.0 * t) * env * 0.6)
    return out


def pickup():
    """Collecting gold: a bright two-note rise (E6 -> A6)."""
    out = []
    for i in range(int(RATE * 0.16)):
        t = i / RATE
        freq = 1318.5 if t < 0.07 else 1760.0
        env = math.exp(-t * 14.0)
        out.append(math.sin(2.0 * math.pi * freq * t) * env * 0.7)
    return out


def win():
    """World cleared: a small rising arpeggio (C5 E5 G5 C6)."""
    notes = [523.25, 659.25, 783.99, 1046.5]
    out = [0.0] * int(RATE * 0.6)
    for k, f in enumerate(notes):
        start = int(RATE * 0.09 * k)
        for i in range(start, len(out)):
            t = (i - start) / RATE
            out[i] += math.sin(2.0 * math.pi * f * t) * math.exp(-t * 7.0) * 0.5
    return out


def music():
    """A 16-second ambient loop at 22050 Hz (half rate keeps the
    file small; pads don't need the top octave).

    Four pad chords (Am7, Fmaj7, Cmaj7, G6), four seconds each,
    crossfaded with wrap-around envelopes so the loop point is
    seamless by construction, plus sparse pentatonic plucks that
    all decay before the seam.
    """
    rate = 22050
    dur = 16.0
    n = int(rate * dur)
    chords = [
        [110.00, 164.81, 196.00, 261.63],  # Am7
        [87.31, 130.81, 164.81, 220.00],   # Fmaj7
        [130.81, 196.00, 246.94, 329.63],  # Cmaj7
        [98.00, 146.83, 246.94, 329.63],   # G6
    ]
    slot = dur / len(chords)
    fade = 0.6

    def chord_gain(t, i):
        # Periodic raised-cosine window for chord i, wrapping at dur.
        start = i * slot
        # distance into this chord's slot, modulo the loop
        local = (t - start) % dur
        if local < slot - 0.0:
            g = 1.0
            if local < fade:
                g = 0.5 - 0.5 * math.cos(math.pi * local / fade)
            tail = local - (slot - fade)
            if tail > 0:
                g = min(g, 0.5 + 0.5 * math.cos(math.pi * tail / fade))
            return g
        # in the fade-out spill past the slot (wraps to chord i+1)
        spill = local - slot
        if spill < fade:
            return 0.5 + 0.5 * math.cos(math.pi * spill / fade)
        return 0.0

    plucks = [  # (time, freq)
        (1.0, 440.00), (3.5, 523.25), (5.0, 659.25), (7.5, 587.33),
        (9.0, 783.99), (11.0, 659.25), (13.0, 523.25), (13.8, 440.00),
    ]
    out = [0.0] * n
    for i in range(n):
        t = i / rate
        s = 0.0
        for ci, chord in enumerate(chords):
            g = chord_gain(t, ci)
            if g <= 0.0:
                continue
            for f in chord:
                # two partials per note, gentle periodic tremolo
                trem = 0.85 + 0.15 * math.sin(2.0 * math.pi * 2.0 * t / dur + f)
                s += g * trem * (math.sin(2.0 * math.pi * f * t)
                                 + 0.3 * math.sin(4.0 * math.pi * f * t))
        s *= 0.055
        for pt, pf in plucks:
            dt = t - pt
            if 0.0 <= dt < 2.0:
                tau = 0.5 if pt > 13.5 else 1.1
                s += 0.16 * math.exp(-dt / tau) * (
                    math.sin(2.0 * math.pi * pf * t)
                    + 0.4 * math.sin(4.0 * math.pi * pf * t))
        out[i] = s
    # write at this function's own rate
    import wave as _w
    os.makedirs("assets", exist_ok=True)
    with _w.open("assets/music.wav", "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(b"".join(
            struct.pack("<h", max(-32767, min(32767, int(v * 32767))))
            for v in out))
    print("wrote assets/music.wav")


if __name__ == "__main__":
    random.seed(20261115)  # the sounds are deterministic too
    write_wav("assets/step.wav", step())
    write_wav("assets/splash.wav", splash())
    write_wav("assets/bump.wav", bump())
    write_wav("assets/click.wav", click())
    write_wav("assets/pickup.wav", pickup())
    write_wav("assets/win.wav", win())
    music()
