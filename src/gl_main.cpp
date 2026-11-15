#include <SFML/Window.hpp>
#include "GLLoader.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include "Mat4.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// MyGL (Lessons 35-38) — the raw OpenGL host. Lesson 38 is the
// merger: the Gold Rush tile world (same Game, same seed, same
// palette) extruded into a 3D heightfield — one flat top per tile at
// its type's height, skirt walls on every step — flown over with
// Lesson 37's camera. Controls: WASD move, Q/E down/up, arrows
// look, Esc quit.

namespace
{

const char* kVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 uMVP;
out vec3 vColor;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

const char* kFragmentShader = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(vColor, 1.0);
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

// The heightfield mesh: position + colour per vertex, a plain
// triangle list. Tops take the tile's palette colour; skirts are
// the same colour darkened, a cheap depth cue until real lighting.
std::vector<float> buildTerrain(const Game& game)
{
    std::vector<float> verts;
    auto push = [&verts](float x, float y, float z, float r, float g, float b) {
        verts.insert(verts.end(), {x, y, z, r, g, b});
    };
    // Corners in perimeter order; emits (a,b,c) and (a,c,d).
    auto quad = [&push](const float a[3], const float b[3], const float c[3],
                        const float d[3], float r, float g, float bl) {
        push(a[0], a[1], a[2], r, g, bl);
        push(b[0], b[1], b[2], r, g, bl);
        push(c[0], c[1], c[2], r, g, bl);
        push(a[0], a[1], a[2], r, g, bl);
        push(c[0], c[1], c[2], r, g, bl);
        push(d[0], d[1], d[2], r, g, bl);
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
            quad(A, D, C, B, r, g, b); // top, facing +Y

            const float sr = r * 0.72f;
            const float sg = g * 0.72f;
            const float sb = b * 0.72f;
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
                quad(p, q, s, t2, sr, sg, sb);
            }
            nh = neighbourHeight(tx - 1, ty);
            if (nh < height)
            {
                const float p[3] = {x0, nh, z0};
                const float q[3] = {x0, nh, z1};
                const float s[3] = {x0, height, z1};
                const float t2[3] = {x0, height, z0};
                quad(p, q, s, t2, sr, sg, sb);
            }
            nh = neighbourHeight(tx, ty + 1);
            if (nh < height)
            {
                const float p[3] = {x0, nh, z1};
                const float q[3] = {x1, nh, z1};
                const float s[3] = {x1, height, z1};
                const float t2[3] = {x0, height, z1};
                quad(p, q, s, t2, sr, sg, sb);
            }
            nh = neighbourHeight(tx, ty - 1);
            if (nh < height)
            {
                const float p[3] = {x1, nh, z0};
                const float q[3] = {x0, nh, z0};
                const float s[3] = {x0, height, z0};
                const float t2[3] = {x1, height, z0};
                quad(p, q, s, t2, sr, sg, sb);
            }
        }
    }
    return verts;
}

} // namespace

int main()
{
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

    GLuint vertex = compileShader(GL_VERTEX_SHADER, kVertexShader, "Vertex");
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, kFragmentShader, "Fragment");
    if (vertex == 0 || fragment == 0)
        return 1;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE)
    {
        std::array<char, 1024> log{};
        glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        Log::error(std::string("Program failed to link: ") + log.data());
        return 1;
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint mvpLocation = glGetUniformLocation(program, "uMVP");
    if (mvpLocation < 0)
    {
        Log::error("Uniform uMVP not found in the shader program");
        return 1;
    }
    Log::info("Shader program linked; uMVP located");

    // The world, from the same Game the 2D hosts play (default seed
    // 12345 — the Gold Rush world), extruded by tile type.
    Game game;
    std::vector<float> terrain = buildTerrain(game);
    const GLsizei terrainVertexCount = static_cast<GLsizei>(terrain.size() / 6);
    Log::info("Terrain built: " + std::to_string(terrainVertexCount / 3) +
              " triangles from a " + std::to_string(game.tilesX()) + "x" +
              std::to_string(game.tilesY()) + " tile grid");

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(terrain.size() * sizeof(float)),
                 terrain.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          reinterpret_cast<const void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vao);
        // The terrain is built in world coordinates, so its model
        // matrix is the identity — one draw for the whole world.
        Mat4 mvp = projection * view;
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp.data());
        glDrawArrays(GL_TRIANGLES, 0, terrainVertexCount);
        window.display();
    }
    Log::info("Camera stopped at (" + std::to_string(cameraPos.x) + ", " +
              std::to_string(cameraPos.y) + ", " + std::to_string(cameraPos.z) + ")");
    return 0;
}
