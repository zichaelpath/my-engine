# Devlog 018 — Scene Save/Load (Arc 4 Opens)

## Goal

Serialise the world to a file of our own design and load it back identically. The format is the lesson: what a scene *is*, decided explicitly.

## Concept

A scene file is a decision about what state matters. This engine's answer is small, and Arc 2 is why:

- **Save the recipe, not the meal.** The terrain — 4,800 tiles, 24 blocks, 40 dots — is a deterministic function of (seed, noiseScale). Storing it would be storing a computation's output; the file stores the *inputs* instead: two lines. Everything else regenerates exactly, in any host, forever. This is the compound interest on Lesson 10's determinism: saves stay tiny and can never disagree with the generator.
- **Entities are data, so entities are text.** Each entity is one pipe-separated line: name, position, size, colour, the controlled/solid flags, sprite id. Human-readable on purpose — a save you can open in Notepad and *edit* is a debugging tool, not just storage. The format carries a `version=1` line from day one, because the cheapest time to add versioning is before there's a second version.
- **Load whole or not at all.** The parser reads into temporaries and only touches the live world after the file parses clean. A corrupt line earns a warning with its line number and is skipped (the Config lesson's philosophy); a wrong version or an entity-less file refuses outright. A half-loaded world — new seed, old entities — is the failure mode this structure exists to prevent.
- **Honest limit:** the file stores the recipe, not stream *history*. If you reshuffle the terrain (advancing the RNG streams past their seed state) and then save, the load regenerates the seed's original layout, not the reshuffled one. Reshuffle state is session state in v1; the fix (store stream draw counts, or store the blocks explicitly) belongs to whichever lesson first needs it.

## Code built

- `Game::saveScene(path)` / `Game::loadScene(path)`: the text format above, written/read with the engine's Log for every outcome. `regenerateWorld()` extracted from `setSeed` so loading and reseeding share one regeneration path.
- Both hosts: **F5 saves, F9 loads** `scene.txt` beside the executable (quicksave/quickload, the way games have done it for decades). The editor also gains Save/Load buttons in a Scene section of the debug panel, and loading re-syncs the Procgen panel's seed/scale fields to the loaded values (which required hoisting that panel state out of the panel — a small, honest refactor).

## What broke

Nothing in the build. One near-miss in the writing: `setSeed` had quietly grown a `Random::reseed` call and different log wording since it was first written (the user's own edits), so the planned refactor had to be re-aimed at the code as it actually stands — read before you refactor, again.

## Verification

Built clean on the Linux rig. The round-trip was driven in the editor (`docs/shots/lesson-18-editor.png`): the player was walked diagonally to **(542, 455)**, F5 saved, the player was walked ~200 px away, and F9 loaded — the debug panel reads **(542, 455)** again, exactly. The console tells the same story in order: "Scene saved to scene.txt (3 entities)", the regeneration lines as the recipe re-executed (field 80x60, terrain 24, decor 40), then "Scene loaded from scene.txt (3 entities, seed 12345)". And the file itself is the third witness — the entire world, nine lines:

```
# MyEngine scene
version=1
seed=12345
noiseScale=0.0099999998
entities=3
entity=Player|542.27893|455.21237|50|50|80|200|120|1|1|0
entity=Crate|200|150|40|40|220|140|60|0|1|1
entity=Crate|560|380|40|40|220|140|60|0|1|1
```

## Explain-it check

_(Post-handover format: why is the scene file ~10 lines instead of ~5,000? What does "load whole or not at all" protect against, concretely? Why does the version line exist in the very first version of the format? — the answers live in the Concept section above.)_

## Next step

Lesson 19 — resources: textures stop being only code-generated. A small resource manager loads image files from an `assets/` folder with caching and graceful fallback, and the sprite sheet gains a file-backed sibling.
