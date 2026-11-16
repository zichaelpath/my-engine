#include "TerrainMesh.hpp"

#include "Game.hpp"

// Height per tile type (Lesson 38): water sits below the land it
// laps against, rock stands highest. The numbers are world units —
// one tile is one unit across.
float tileHeight(int id)
{
    switch (id)
    {
        case tiles::DeepWater: return -0.55f;
        case tiles::Water: return -0.25f;
        case tiles::Sand: return 0.05f;
        case tiles::Grass: return 0.35f;
        case tiles::Forest: return 0.65f;
        default: return 1.15f; // rock
    }
}

// Specular character per tile type (Lesson 89): the Blinn-Phong
// (shininess exponent, strength). Water is the mirror of this
// world — a high exponent keeps its highlight tight and its
// strength near full; grass barely answers at all.
void tileSpec(int id, float& shininess, float& strength)
{
    switch (id)
    {
        case tiles::DeepWater: shininess = 90.f; strength = 0.95f; break;
        case tiles::Water:     shininess = 64.f; strength = 0.85f; break;
        case tiles::Sand:      shininess = 10.f; strength = 0.20f; break;
        case tiles::Grass:     shininess = 6.f;  strength = 0.12f; break;
        case tiles::Forest:    shininess = 8.f;  strength = 0.15f; break;
        default:               shininess = 20.f; strength = 0.30f; break; // rock
    }
}

// The heightfield mesh: position + normal + colour per vertex, a
// plain triangle list. Every face of this mesh is axis-aligned, so
// its normal is known exactly at build time — tops point +Y, skirts
// point along their axis. (A general mesh would compute normals
// from cross products of its triangles; here that would only
// rediscover what the builder already knows.)
std::vector<float> buildTerrain(const Game& game)
{
    std::vector<float> verts;
    auto push = [&verts](float x, float y, float z, float nx, float ny, float nz,
                         float r, float g, float b, float shin, float strength) {
        verts.insert(verts.end(),
                     {x, y, z, nx, ny, nz, r, g, b, shin, strength});
    };
    // Corners in perimeter order; emits (a,b,c) and (a,c,d), all
    // with the face's normal.
    auto quad = [&push](const float a[3], const float b[3], const float c[3],
                        const float d[3], float nx, float ny, float nz,
                        float r, float g, float bl, float shin,
                        float strength) {
        push(a[0], a[1], a[2], nx, ny, nz, r, g, bl, shin, strength);
        push(b[0], b[1], b[2], nx, ny, nz, r, g, bl, shin, strength);
        push(c[0], c[1], c[2], nx, ny, nz, r, g, bl, shin, strength);
        push(a[0], a[1], a[2], nx, ny, nz, r, g, bl, shin, strength);
        push(c[0], c[1], c[2], nx, ny, nz, r, g, bl, shin, strength);
        push(d[0], d[1], d[2], nx, ny, nz, r, g, bl, shin, strength);
    };

    const float base = -0.8f; // skirt bottom at the map's edge
    const int w = game.tilesX();
    const int h = game.tilesY();
    for (int ty = 0; ty < h; ++ty)
    {
        for (int tx = 0; tx < w; ++tx)
        {
            int id = game.tileAt(tx, ty);
            float height = tileHeight(id);
            float shin = 0.f, specStrength = 0.f;
            tileSpec(id, shin, specStrength);
            sf::Color c = Game::tileColor(id);
            float r = c.r / 255.f;
            float g = c.g / 255.f;
            float b = c.b / 255.f;
            float x0 = static_cast<float>(tx) - w / 2.f;
            float x1 = x0 + 1.f;
            float z0 = static_cast<float>(ty) - h / 2.f;
            float z1 = z0 + 1.f;

            const float A[3] = {x0, height, z0};
            const float B[3] = {x1, height, z0};
            const float C[3] = {x1, height, z1};
            const float D[3] = {x0, height, z1};
            quad(A, D, C, B, 0.f, 1.f, 0.f, r, g, b, shin, specStrength); // top, facing +Y

            auto neighbourHeight = [&](int nx, int ny) {
                int nid = game.tileAt(nx, ny);
                return nid < 0 ? base : tileHeight(nid);
            };
            // A skirt wherever this tile stands above its neighbour
            // (the higher tile owns the wall, so each step gets one).
            float nh = neighbourHeight(tx + 1, ty);
            if (nh < height)
            {
                const float p[3] = {x1, nh, z1};
                const float q[3] = {x1, nh, z0};
                const float s[3] = {x1, height, z0};
                const float t2[3] = {x1, height, z1};
                quad(p, q, s, t2, 1.f, 0.f, 0.f, r, g, b, shin, specStrength);
            }
            nh = neighbourHeight(tx - 1, ty);
            if (nh < height)
            {
                const float p[3] = {x0, nh, z0};
                const float q[3] = {x0, nh, z1};
                const float s[3] = {x0, height, z1};
                const float t2[3] = {x0, height, z0};
                quad(p, q, s, t2, -1.f, 0.f, 0.f, r, g, b, shin, specStrength);
            }
            nh = neighbourHeight(tx, ty + 1);
            if (nh < height)
            {
                const float p[3] = {x0, nh, z1};
                const float q[3] = {x1, nh, z1};
                const float s[3] = {x1, height, z1};
                const float t2[3] = {x0, height, z1};
                quad(p, q, s, t2, 0.f, 0.f, 1.f, r, g, b, shin, specStrength);
            }
            nh = neighbourHeight(tx, ty - 1);
            if (nh < height)
            {
                const float p[3] = {x1, nh, z0};
                const float q[3] = {x0, nh, z0};
                const float s[3] = {x0, height, z0};
                const float t2[3] = {x1, height, z0};
                quad(p, q, s, t2, 0.f, 0.f, -1.f, r, g, b, shin, specStrength);
            }
        }
    }
    return verts;
}

