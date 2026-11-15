#include <SFML/Window.hpp>
#include "GLLoader.hpp"
#include "Log.hpp"
#include <array>
#include <string>

// MyGL (Lesson 35) — the engine's third host and Arc 8's beachhead:
// a raw OpenGL 3.3 core window beside the SFML-rendered ones. SFML
// is demoted to what it is good at here — window, context, events —
// and every pixel is drawn by our own shader pipeline: source ->
// compile -> link -> buffers -> draw. The 2D hosts are untouched.

namespace
{

const char* kVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vColor;
void main()
{
    gl_Position = vec4(aPos, 1.0);
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

// Compiles one shader stage; on failure, logs the driver's own
// message (the info log) and returns 0. Shader bugs are found in
// these logs or not at all — there is no stack trace in GLSL land.
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
    Log::info("Shader program linked");

    // One triangle, position and colour interleaved per vertex. The
    // fragment colour is interpolated across the face by the GPU —
    // the gradient is the proof the whole pipeline is alive.
    const std::array<float, 18> vertices = {
        // position          colour
        -0.6f, -0.5f, 0.0f, 1.0f, 0.25f, 0.2f, // bottom-left, red
        0.6f, -0.5f, 0.0f, 0.25f, 1.0f, 0.35f, // bottom-right, green
        0.0f, 0.55f, 0.0f, 0.3f, 0.45f, 1.0f,  // top, blue
    };
    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          reinterpret_cast<const void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

        sf::Vector2u size = window.getSize();
        glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        window.display();
    }
    return 0;
}
