#include "GLProgram.hpp"

#include "GLLoader.hpp"
#include "Log.hpp"

#include <array>

namespace
{

GLuint compileShader(GLenum type, const char* source,
                     const char* stageName)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE)
    {
        std::array<char, 1024> log{};
        glGetShaderInfoLog(shader,
                           static_cast<GLsizei>(log.size()), nullptr,
                           log.data());
        Log::error(std::string(stageName) +
                   " shader failed to compile: " + log.data());
        return 0;
    }
    Log::info(std::string(stageName) + " shader compiled");
    return shader;
}

} // namespace

GLuint compileProgram(const char* vertexSource,
                      const char* fragmentSource)
{
    GLuint vertex =
        compileShader(GL_VERTEX_SHADER, vertexSource, "Vertex");
    GLuint fragment =
        compileShader(GL_FRAGMENT_SHADER, fragmentSource, "Fragment");
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
        glGetProgramInfoLog(program,
                            static_cast<GLsizei>(log.size()), nullptr,
                            log.data());
        Log::error(std::string("Program failed to link: ") +
                   log.data());
        if (glDeleteProgram != nullptr)
            glDeleteProgram(program);
        return 0;
    }
    return program;
}
