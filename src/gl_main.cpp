#include <SFML/Window.hpp>
#include "GLLoader.hpp"
#include "Log.hpp"
#include "Mat4.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

// MyGL (Lessons 35-37) — the raw OpenGL host. Lesson 37 adds a
// flyable camera (WASD + Q/E, arrow keys look) built on Mat4::lookAt,
// and a field of 25 spinning cubes so the flight has something to
// read against. Controls: WASD move, Q/E down/up, arrows look,
// Esc quit.

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

    // The cube: 8 corners, each coloured by its own coordinates
    // (position + 0.5), so the whole RGB space shows on the surface.
    // Faces share corners through the index buffer — 36 indices
    // instead of 36 duplicated vertices.
    const std::array<float, 48> vertices = {
        // position              colour (position + 0.5)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
    };
    // Outward-facing, counter-clockwise seen from outside each face.
    const std::array<std::uint32_t, 36> indices = {
        4, 5, 6, 4, 6, 7, // front (+z)
        1, 0, 3, 1, 3, 2, // back (-z)
        5, 1, 2, 5, 2, 6, // right (+x)
        0, 4, 7, 0, 7, 3, // left (-x)
        7, 6, 2, 7, 2, 3, // top (+y)
        0, 1, 5, 0, 5, 4, // bottom (-y)
    };
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
                 indices.data(), GL_STATIC_DRAW);
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
    Vec3 cameraPos{0.f, 1.4f, 2.5f};
    float yaw = 0.f;
    float pitch = -0.15f;
    auto forward = [&]() {
        return Vec3{-std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                    -std::cos(yaw) * std::cos(pitch)};
    };

    sf::Clock clock;
    sf::Clock elapsed;
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
        float t = elapsed.getElapsedTime().asSeconds();

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

        const float moveSpeed = 3.f;
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
        Mat4 projection = Mat4::perspective(60.f * 3.14159265f / 180.f, aspect, 0.1f, 100.f);

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vao);
        // One VAO, 25 draws: the model matrix is the only thing that
        // changes between cubes — place, then spin with a per-cube
        // phase so the field doesn't move in lockstep.
        for (int gx = -2; gx <= 2; ++gx)
        {
            for (int gz = -2; gz <= 2; ++gz)
            {
                float phase = static_cast<float>((gx + 2) * 5 + (gz + 2)) * 0.37f;
                Mat4 model = Mat4::translation(static_cast<float>(gx) * 2.2f, 0.f,
                                               -6.f + static_cast<float>(gz) * 2.2f) *
                             Mat4::rotationY(t * 0.7f + phase) *
                             Mat4::rotationX(t * 0.4f + phase);
                Mat4 mvp = projection * view * model;
                glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp.data());
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            }
        }
        window.display();
    }
    Log::info("Camera stopped at (" + std::to_string(cameraPos.x) + ", " +
              std::to_string(cameraPos.y) + ", " + std::to_string(cameraPos.z) + ")");
    return 0;
}
