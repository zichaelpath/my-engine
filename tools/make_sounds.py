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


if __name__ == "__main__":
    random.seed(20261115)  # the sounds are deterministic too
    write_wav("assets/step.wav", step())
    write_wav("assets/splash.wav", splash())
