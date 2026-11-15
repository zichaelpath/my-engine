#include <SFML/Window.hpp>
#include "GLLoader.hpp"
#include "Log.hpp"
#include "Mat4.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

// MyGL (Lessons 35-36) — the raw OpenGL host. Lesson 36 grows the
// gradient triangle into a spinning cube: indexed geometry (8 shared
// corners, 36 indices), the depth test, and the MVP chain — model
// rotation, view translation, perspective projection — computed per
// frame by the engine's own Mat4 and uploaded as a uniform.

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
        float t = clock.getElapsedTime().asSeconds();

        sf::Vector2u size = window.getSize();
        float aspect = size.y > 0 ? static_cast<float>(size.x) / static_cast<float>(size.y)
                                  : 1.f;
        glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

        // The chain, read right to left: the cube spins in its own
        // space (model), the world slides back so the camera can see
        // it (view), and the frustum squeezes it into the cube of
        // clip space (projection).
        Mat4 model = Mat4::rotationY(t * 0.9f) * Mat4::rotationX(t * 0.6f);
        Mat4 view = Mat4::translation(0.f, 0.f, -2.6f);
        Mat4 projection = Mat4::perspective(60.f * 3.14159265f / 180.f, aspect, 0.1f, 100.f);
        Mat4 mvp = projection * view * model;

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp.data());
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        window.display();
    }
    return 0;
}
