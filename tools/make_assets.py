#!/usr/bin/env python3
"""Generates the engine's file-backed assets (currently: the crate).

Pure standard library: writes an uncompressed 32bpp TGA, which SFML
loads natively. Run from the repo root:  python3 tools/make_assets.py
The generated files live in assets/ and are committed — this script
is their source, kept so the art is reproducible and editable.
"""

import os
import struct

SIZE = 32


def write_tga(path, pixels):
    """pixels: list of rows (top first), each a list of (r, g, b, a)."""
    header = struct.pack(
        "<BBBHHBHHHHBB",
        0,          # id length
        0,          # no colour map
        2,          # uncompressed true-colour
        0, 0, 0,    # colour map spec
        0, 0,       # x/y origin
        SIZE, SIZE, # width, height
        32,         # bits per pixel
        0x28,       # 8 alpha bits, top-left origin
    )
    body = bytearray()
    for row in pixels:
        for (r, g, b, a) in row:
            body += bytes((b, g, r, a))  # TGA stores BGRA
    with open(path, "wb") as f:
        f.write(header)
        f.write(body)


def crate_pixels():
    wood = (172, 110, 50, 255)
    wood_light = (196, 132, 66, 255)
    wood_dark = (138, 84, 36, 255)
    iron = (74, 74, 84, 255)
    iron_dark = (48, 48, 56, 255)
    outline = (96, 58, 26, 255)

    px = [[wood] * SIZE for _ in range(SIZE)]
    # Vertical planks with seams and a light streak each.
    for x in range(SIZE):
        for y in range(SIZE):
            if x % 8 == 7:
                px[y][x] = wood_dark
            elif x % 8 == 1:
                px[y][x] = wood_light
    # Iron bands top and bottom, with rivets.
    for x in range(SIZE):
        for y in (0, 1, 2, SIZE - 3, SIZE - 2, SIZE - 1):
            px[y][x] = iron
    for x in (3, 15, 27):
        for y in (1, SIZE - 2):
            px[y][x] = iron_dark
    # Corner brackets.
    for cx, cy in ((0, 0), (SIZE - 4, 0), (0, SIZE - 4), (SIZE - 4, SIZE - 4)):
        for dx in range(4):
            for dy in range(4):
                if dx + dy < 4 or (cx > 0 and (3 - dx) + dy < 4) or \
                   (cy > 0 and dx + (3 - dy) < 4) or \
                   (cx > 0 and cy > 0 and (3 - dx) + (3 - dy) < 4):
                    px[cy + dy][cx + dx] = iron
    # Outline.
    for i in range(SIZE):
        px[0][i] = outline
        px[SIZE - 1][i] = outline
        px[i][0] = outline
        px[i][SIZE - 1] = outline
    return px


def main():
    os.makedirs("assets", exist_ok=True)
    write_tga(os.path.join("assets", "crate.tga"), crate_pixels())
    print("wrote assets/crate.tga")


if __name__ == "__main__":
    main()
