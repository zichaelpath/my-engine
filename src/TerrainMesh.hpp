#pragma once

#include <vector>

class Game;

// TerrainMesh (extracted in Lesson 93 from the GL host) — the
// heightfield rules both 3D views share: how tall each tile
// type stands, its specular character, and the triangle soup
// built from a Game's tile grid. CPU-side only, so the harness
// pins it and any host (the standalone MyGL, the editor's 3D
// viewport) builds the identical world.
//
// Vertex layout (11 floats): position (3) + normal (3) +
// colour (3) + specular pair (2). One tile is one world unit;
// the grid is centred on the origin.
float tileHeight(int id);
void tileSpec(int id, float& shininess, float& strength);
std::vector<float> buildTerrain(const Game& game);
