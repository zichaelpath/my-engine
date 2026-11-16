#include <SFML/Window.hpp>
#include "GLLoader.hpp"
#include "Mesh.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include "SpriteSheet.hpp"
#include "Version.hpp"
#include "Mat4.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// MyGL (Lessons 35-40) — the raw OpenGL host. Lesson 40 closes
// Arc 8 with texture: a second shader program samples the crate
// texture (assets/crate.tga, the same file the 2D game loads), and
// the Game's own crate entities stand on the terrain at their
// world positions, UV-mapped and sun-lit. Controls: WASD move,
// Q/E down/up, arrows look, Esc quit.
//
// Phase 2's Arc 9 (Lessons 89+) grows this host from demo to
// renderer. Lesson 89: Blinn-Phong specular — highlights need
// the world position and the camera, so both programs now carry
// a model matrix and a camera uniform alongside the MVP.

namespace
{

const char* kVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aSpec;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vColor;
out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vSpec;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    vColor = aColor;
    vNormal = mat3(uModel) * aNormal;
    vWorldPos = (uModel * vec4(aPos, 1.0)).xyz;
    vSpec = aSpec;
}
)";

const char* kFragmentShader = R"(
#version 330 core
in vec3 vColor;
in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vSpec;
uniform vec3 uSunDir;
uniform vec3 uCameraPos;
uniform mat4 uLightSpace;
uniform sampler2D uShadowMap;
out vec4 FragColor;
float shadowAmount(vec3 worldPos, vec3 n, vec3 sunDir)
{
    vec4 ls = uLightSpace * vec4(worldPos, 1.0);
    vec3 proj = ls.xyz / ls.w * 0.5 + 0.5;
    if (proj.z > 1.0)
        return 0.0;
    float bias = max(0.004 * (1.0 - dot(n, sunDir)), 0.0008);
    float shadow = 0.0;
    vec2 texel = 1.0 / vec2(2048.0);
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
        {
            float closest = texture(uShadowMap,
                proj.xy + vec2(float(x), float(y)) * texel).r;
            shadow += (proj.z - bias > closest) ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}
void main()
{
    // Lambert diffuse: a face square to the sun takes full light,
    // a face edge-on or turned away falls to the ambient floor.
    // A shadowed fragment (Lesson 90) keeps only the ambient:
    // the sun's diffuse AND its specular are both direct light.
    vec3 n = normalize(vNormal);
    float diffuse = max(dot(n, uSunDir), 0.0);
    float direct = 1.0 - shadowAmount(vWorldPos, n, uSunDir);
    float light = 0.35 + 0.65 * diffuse * direct;
    // Blinn-Phong specular (Lesson 89): the halfway vector
    // between sun and view directions — a tight, bright highlight
    // where the surface mirrors the sun into the camera. vSpec
    // carries this material's (shininess, strength).
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 halfVec = normalize(uSunDir + viewDir);
    float spec = pow(max(dot(n, halfVec), 0.0), vSpec.x) * vSpec.y
                 * direct;
    FragColor = vec4(vColor * light + spec, 1.0);
}
)";

// The shadow test (Lesson 90) lives inside both fragment
// shaders below — GLSL has no includes, so the function is
// written out in each: project the fragment into the sun's clip
// space, compare its depth against the shadow map's stored
// depth with a 3x3 PCF average. 0 = fully lit, 1 = fully
// shadowed. The bias grows on grazing faces, which is where
// shadow acne lives.

// The textured program (Lesson 40): same sun, but the base colour
// comes from a texture sample instead of a vertex colour.
const char* kTexVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vNormal;
out vec2 vUV;
out vec3 vWorldPos;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    vWorldPos = (uModel * vec4(aPos, 1.0)).xyz;
}
)";

const char* kTexFragmentShader = R"(
#version 330 core
in vec3 vNormal;
in vec2 vUV;
in vec3 vWorldPos;
uniform vec3 uSunDir;
uniform vec3 uCameraPos;
uniform float uShininess;
uniform float uSpecStrength;
uniform sampler2D uTexture;
uniform mat4 uLightSpace;
uniform sampler2D uShadowMap;
out vec4 FragColor;
float shadowAmount(vec3 worldPos, vec3 n, vec3 sunDir)
{
    vec4 ls = uLightSpace * vec4(worldPos, 1.0);
    vec3 proj = ls.xyz / ls.w * 0.5 + 0.5;
    if (proj.z > 1.0)
        return 0.0;
    float bias = max(0.004 * (1.0 - dot(n, sunDir)), 0.0008);
    float shadow = 0.0;
    vec2 texel = 1.0 / vec2(2048.0);
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
        {
            float closest = texture(uShadowMap,
                proj.xy + vec2(float(x), float(y)) * texel).r;
            shadow += (proj.z - bias > closest) ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}
void main()
{
    vec3 n = normalize(vNormal);
    float diffuse = max(dot(n, uSunDir), 0.0);
    float direct = 1.0 - shadowAmount(vWorldPos, n, uSunDir);
    float light = 0.35 + 0.65 * diffuse * direct;
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 halfVec = normalize(uSunDir + viewDir);
    float spec = pow(max(dot(n, halfVec), 0.0), uShininess)
                 * uSpecStrength * direct;
    FragColor = vec4(texture(uTexture, vUV).rgb * light + spec,
                     1.0);
}
)";

// The depth program (Lesson 90): position only, transformed by
// the sun's light-space matrix — its fragment shader writes
// nothing because the depth buffer is the whole output.
const char* kDepthVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char* kDepthFragmentShader = R"(
#version 330 core
void main()
{
}
)";

GLuint compileShader(GLenum type, const char* source, const char* stageName)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE)
    {
        std::array<char, 1024> log{};
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        Log::error(std::string(stageName) + " shader failed to compile: " + log.data());
        return 0;
    }
    Log::info(std::string(stageName) + " shader compiled");
    return shader;
}

// Compiles and links a full program; 0 on any failure (the stage
// logs carry the details).
GLuint createProgram(const char* vertexSource, const char* fragmentSource)
{
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource, "Vertex");
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource, "Fragment");
    if (vertex == 0 || fragment == 0)
        return 0;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (linked == GL_FALSE)
    {
        std::array<char, 1024> log{};
        glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        Log::error(std::string("Program failed to link: ") + log.data());
        return 0;
    }
    return program;
}

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

} // namespace

int main()
{
    Log::info(std::string("MyEngine ") + engine::VERSION + " - GL host");
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;

    sf::Window window(sf::VideoMode({800, 600}), "MyEngine GL",
                      sf::State::Windowed, settings);
    window.setFramerateLimit(60);
    if (!window.setActive(true))
    {
        Log::error("Could not activate the OpenGL context");
        return 1;
    }
    if (!loadGLFunctions())
        return 1;
    Log::info(std::string("OpenGL version: ") +
              reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    GLuint program = createProgram(kVertexShader, kFragmentShader);
    GLuint texProgram = createProgram(kTexVertexShader, kTexFragmentShader);
    GLuint depthProgram = createProgram(kDepthVertexShader,
                                        kDepthFragmentShader);
    if (program == 0 || texProgram == 0 || depthProgram == 0)
        return 1;
    GLint mvpLocation = glGetUniformLocation(program, "uMVP");
    GLint modelLocation = glGetUniformLocation(program, "uModel");
    GLint sunLocation = glGetUniformLocation(program, "uSunDir");
    GLint camLocation = glGetUniformLocation(program, "uCameraPos");
    GLint lightSpaceLocation = glGetUniformLocation(program, "uLightSpace");
    GLint shadowSamplerLocation = glGetUniformLocation(program, "uShadowMap");
    GLint texMvpLocation = glGetUniformLocation(texProgram, "uMVP");
    GLint texModelLocation = glGetUniformLocation(texProgram, "uModel");
    GLint texSunLocation = glGetUniformLocation(texProgram, "uSunDir");
    GLint texCamLocation = glGetUniformLocation(texProgram, "uCameraPos");
    GLint texShineLocation = glGetUniformLocation(texProgram, "uShininess");
    GLint texSpecLocation = glGetUniformLocation(texProgram, "uSpecStrength");
    GLint texSamplerLocation = glGetUniformLocation(texProgram, "uTexture");
    GLint texLightSpaceLocation =
        glGetUniformLocation(texProgram, "uLightSpace");
    GLint texShadowSamplerLocation =
        glGetUniformLocation(texProgram, "uShadowMap");
    GLint depthMvpLocation = glGetUniformLocation(depthProgram, "uMVP");
    if (mvpLocation < 0 || modelLocation < 0 || sunLocation < 0 ||
        camLocation < 0 || lightSpaceLocation < 0 ||
        shadowSamplerLocation < 0 || texMvpLocation < 0 ||
        texModelLocation < 0 || texSunLocation < 0 ||
        texCamLocation < 0 || texShineLocation < 0 ||
        texSpecLocation < 0 || texSamplerLocation < 0 ||
        texLightSpaceLocation < 0 || texShadowSamplerLocation < 0 ||
        depthMvpLocation < 0)
    {
        Log::error("A required uniform was not found in a shader program");
        return 1;
    }
    // The sun: low in the west, slightly south — direction TOWARD
    // it, normalized once here; the shaders dot it with each face's
    // normal.
    Vec3 sunDir = normalized(Vec3{-0.55f, 0.75f, 0.35f});
    Log::info("Shader programs linked; uniforms located");

    // The shadow camera (Lesson 90): an orthographic box sitting
    // on the sun's line, looking at the world's centre. Its
    // matrix is constant — the sun never moves — so the depth
    // pass and both lighting programs share this one lightSpace.
    const Mat4 lightSpace =
        Mat4::orthographic(-60.f, 60.f, -60.f, 60.f, 1.f, 160.f) *
        Mat4::lookAt(sunDir * 70.f, Vec3{0.f, 0.f, 0.f},
                     Vec3{0.f, 1.f, 0.f});

    // The shadow map: a depth texture on its own framebuffer.
    // Nothing draws colour into it (GL_NONE draw/read), and its
    // border depth is 1.0 — the farthest possible — so fragments
    // outside the sun's box read as lit, never as shadowed.
    const GLsizei kShadowSize = 2048;
    GLuint shadowMap = 0;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowSize,
                 kShadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_BORDER);
    const float borderDepth[4] = {1.f, 1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
                     borderDepth);
    GLuint shadowFbo = 0;
    glGenFramebuffers(1, &shadowFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE)
    {
        Log::error("Shadow framebuffer is not complete");
        return 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Log::info("Shadow map ready: 2048x2048 depth");

    // The world, from the same Game the 2D hosts play (default seed
    // 12345 — the Gold Rush world), extruded by tile type.
    Game game;
    std::vector<float> terrain = buildTerrain(game);
    Log::info("Terrain built: " +
              std::to_string(terrain.size() / 11 / 3) +
              " triangles from a " + std::to_string(game.tilesX()) +
              "x" + std::to_string(game.tilesY()) + " tile grid");

    // Both meshes are Mesh values now (Lesson 91): the class
    // owns the VAO/VBO/EBO and the attribute contract — position
    // (0), normal (1), colour (2), specular pair (3) for the
    // terrain; position, normal, UV for the crate.
    Mesh terrainMesh(terrain, {{0, 3}, {1, 3}, {2, 3}, {3, 2}});

    // The crates (Lesson 40): mesh, texture, and placements — the
    // placements come from the Game's own entity list, converted
    // from 2D world pixels into terrain space and stood on the
    // height of the tile beneath them. The mesh itself is a
    // loaded asset since Lesson 91 (assets/crate.obj) — the
    // hand-typed cube array is gone.
    std::vector<float> crateVerts;
    std::vector<std::uint32_t> crateIndices;
    Mesh crateMesh;
    if (loadObj("assets/crate.obj", crateVerts, crateIndices))
        crateMesh = Mesh(crateVerts, {{0, 3}, {1, 3}, {2, 2}},
                         crateIndices);

    // The texture: SFML decodes the file (CPU-side loading is its
    // job in every host); the GL texture object is ours. Images load
    // top-row-first and GL textures start at the bottom row, so the
    // image is flipped on the way in. NEAREST magnification keeps
    // the pixel art crisp; mipmaps (generated by the GPU) keep it
    // from shimmering at a distance.
    GLuint crateTexture = 0;
    sf::Image crateImage;
    if (crateImage.loadFromFile("assets/crate.tga"))
    {
        crateImage.flipVertically();
        glGenTextures(1, &crateTexture);
        glBindTexture(GL_TEXTURE_2D, crateTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        sf::Vector2u ts = crateImage.getSize();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(ts.x),
                     static_cast<GLsizei>(ts.y), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     crateImage.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
        Log::info("Crate texture uploaded: " + std::to_string(ts.x) + "x" +
                  std::to_string(ts.y));
    }
    else
    {
        Log::error("Crate texture failed to load: assets/crate.tga (crates skipped)");
    }

    std::vector<Vec3> crateSpots;
    {
        float tileSize = game.worldSize().x / static_cast<float>(game.tilesX());
        for (const Entity& e : game.entities())
        {
            if (e.sprite != sprites::CRATE)
                continue;
            float centreTileX = (e.position.x + e.size.x / 2.f) / tileSize;
            float centreTileY = (e.position.y + e.size.y / 2.f) / tileSize;
            int tx = static_cast<int>(centreTileX);
            int ty = static_cast<int>(centreTileY);
            float ground = tileHeight(game.tileAt(tx, ty));
            crateSpots.push_back({centreTileX - game.tilesX() / 2.f, ground + 0.8f,
                                  centreTileY - game.tilesY() / 2.f});
        }
    }
    Log::info("Crates placed: " + std::to_string(crateSpots.size()));

    glEnable(GL_DEPTH_TEST);

    // The fly camera (Lesson 37). yaw/pitch are the whole state;
    // the forward vector is derived from them with the same rotation
    // conventions as Mat4 (yaw about Y, then pitch about X), so the
    // keys, the movement, and lookAt never disagree.
    Vec3 cameraPos{0.f, 24.f, 38.f};
    float yaw = 0.f;
    float pitch = -0.55f;
    auto forward = [&]() {
        return Vec3{-std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                    -std::cos(yaw) * std::cos(pitch)};
    };

    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
        }
        float dt = clock.restart().asSeconds();

        using Key = sf::Keyboard::Key;
        const float turnSpeed = 1.6f; // radians per second
        if (sf::Keyboard::isKeyPressed(Key::Left))
            yaw += turnSpeed * dt;
        if (sf::Keyboard::isKeyPressed(Key::Right))
            yaw -= turnSpeed * dt;
        if (sf::Keyboard::isKeyPressed(Key::Up))
            pitch += turnSpeed * dt;
        if (sf::Keyboard::isKeyPressed(Key::Down))
            pitch -= turnSpeed * dt;
        const float pitchLimit = 1.45f; // just under straight up/down
        if (pitch > pitchLimit)
            pitch = pitchLimit;
        if (pitch < -pitchLimit)
            pitch = -pitchLimit;

        const float moveSpeed = 10.f;
        Vec3 front = forward();
        Vec3 right = normalized(Vec3{-front.z, 0.f, front.x});
        if (sf::Keyboard::isKeyPressed(Key::W))
            cameraPos += front * (moveSpeed * dt);
        if (sf::Keyboard::isKeyPressed(Key::S))
            cameraPos += front * (-moveSpeed * dt);
        if (sf::Keyboard::isKeyPressed(Key::D))
            cameraPos += right * (moveSpeed * dt);
        if (sf::Keyboard::isKeyPressed(Key::A))
            cameraPos += right * (-moveSpeed * dt);
        if (sf::Keyboard::isKeyPressed(Key::E))
            cameraPos.y += moveSpeed * dt;
        if (sf::Keyboard::isKeyPressed(Key::Q))
            cameraPos.y -= moveSpeed * dt;

        sf::Vector2u size = window.getSize();
        float aspect = size.y > 0 ? static_cast<float>(size.x) / static_cast<float>(size.y)
                                  : 1.f;
        glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

        Mat4 view = Mat4::lookAt(cameraPos, cameraPos + front, Vec3{0.f, 1.f, 0.f});
        Mat4 projection = Mat4::perspective(60.f * 3.14159265f / 180.f, aspect, 0.1f, 300.f);

        // PASS 1 (Lesson 90): depth from the sun. Everything
        // that can cast is drawn into the shadow map through the
        // light-space matrix; the colour pipeline never runs.
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
        glViewport(0, 0, kShadowSize, kShadowSize);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(depthProgram);
        glUniformMatrix4fv(depthMvpLocation, 1, GL_FALSE,
                           lightSpace.data());
        terrainMesh.draw();
        if (crateMesh.valid())
            for (const Vec3& spot : crateSpots)
            {
                Mat4 model = Mat4::translation(spot.x, spot.y, spot.z) *
                             Mat4::scaling(1.6f, 1.6f, 1.6f);
                Mat4 depthMvp = lightSpace * model;
                glUniformMatrix4fv(depthMvpLocation, 1, GL_FALSE,
                                   depthMvp.data());
                crateMesh.draw();
            }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // PASS 2: the lit scene, sampling the shadow map on
        // texture unit 1 (unit 0 stays the crates' colour map).
        glViewport(0, 0, static_cast<GLsizei>(size.x),
                   static_cast<GLsizei>(size.y));
        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        // The terrain is built in world coordinates, so its model
        // matrix is the identity — one draw for the whole world.
        Mat4 mvp = projection * view;
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp.data());
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                           Mat4::identity().data());
        glUniform3f(sunLocation, sunDir.x, sunDir.y, sunDir.z);
        glUniform3f(camLocation, cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(lightSpaceLocation, 1, GL_FALSE,
                           lightSpace.data());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glUniform1i(shadowSamplerLocation, 1);
        glActiveTexture(GL_TEXTURE0);
        terrainMesh.draw();

        // The crates: second program, same sun. Their model is a
        // translation plus a uniform scale — normals survive both
        // (translation never rotates; uniform scale only stretches
        // a normal's length, and the shader normalizes).
        if (crateTexture != 0 && crateMesh.valid())
        {
            glUseProgram(texProgram);
            glBindTexture(GL_TEXTURE_2D, crateTexture);
            glUniform3f(texSunLocation, sunDir.x, sunDir.y, sunDir.z);
            glUniform3f(texCamLocation, cameraPos.x, cameraPos.y,
                        cameraPos.z);
            // The crate's material (Lesson 89): varnished wood —
            // a mid shininess, a modest answer.
            glUniform1f(texShineLocation, 24.f);
            glUniform1f(texSpecLocation, 0.30f);
            glUniform1i(texSamplerLocation, 0);
            glUniformMatrix4fv(texLightSpaceLocation, 1, GL_FALSE,
                               lightSpace.data());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowMap);
            glUniform1i(texShadowSamplerLocation, 1);
            glActiveTexture(GL_TEXTURE0);
            for (const Vec3& spot : crateSpots)
            {
                Mat4 model = Mat4::translation(spot.x, spot.y, spot.z) *
                             Mat4::scaling(1.6f, 1.6f, 1.6f);
                Mat4 crateMvp = projection * view * model;
                glUniformMatrix4fv(texMvpLocation, 1, GL_FALSE, crateMvp.data());
                glUniformMatrix4fv(texModelLocation, 1, GL_FALSE,
                                   model.data());
                crateMesh.draw();
            }
        }
        window.display();
    }
    Log::info("Camera stopped at (" + std::to_string(cameraPos.x) + ", " +
              std::to_string(cameraPos.y) + ", " + std::to_string(cameraPos.z) + ")");
    return 0;
}
