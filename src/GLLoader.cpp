#include "GLLoader.hpp"
#include "Log.hpp"
#include <string>
#include <type_traits>

#ifdef _WIN32
static void* loadProcAddress(const char* name)
{
    // wglGetProcAddress covers everything post-1.1; the opengl32
    // export fallback is belt-and-braces for drivers that return a
    // sentinel instead of nullptr on a miss.
    void* p = reinterpret_cast<void*>(wglGetProcAddress(name));
    if (p == nullptr)
    {
        static HMODULE lib = LoadLibraryA("opengl32.dll");
        if (lib != nullptr)
            p = reinterpret_cast<void*>(GetProcAddress(lib, name));
    }
    return p;
}
#else
#include <GL/glx.h>
static void* loadProcAddress(const char* name)
{
    return reinterpret_cast<void*>(
        glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name)));
}
#endif

bool loadGLFunctions()
{
    bool ok = true;
    auto load = [&ok](const char* name, auto& fn) {
        fn = reinterpret_cast<std::decay_t<decltype(fn)>>(loadProcAddress(name));
        if (fn == nullptr)
        {
            Log::error(std::string("OpenGL function missing: ") + name);
            ok = false;
        }
    };

    load("glCreateShader", glCreateShader);
    load("glShaderSource", glShaderSource);
    load("glCompileShader", glCompileShader);
    load("glGetShaderiv", glGetShaderiv);
    load("glGetShaderInfoLog", glGetShaderInfoLog);
    load("glCreateProgram", glCreateProgram);
    load("glAttachShader", glAttachShader);
    load("glLinkProgram", glLinkProgram);
    load("glGetProgramiv", glGetProgramiv);
    load("glGetProgramInfoLog", glGetProgramInfoLog);
    load("glUseProgram", glUseProgram);
    load("glDeleteShader", glDeleteShader);
    load("glGenVertexArrays", glGenVertexArrays);
    load("glBindVertexArray", glBindVertexArray);
    load("glGenBuffers", glGenBuffers);
    load("glBindBuffer", glBindBuffer);
    load("glBufferData", glBufferData);
    load("glVertexAttribPointer", glVertexAttribPointer);
    load("glEnableVertexAttribArray", glEnableVertexAttribArray);
    load("glGetUniformLocation", glGetUniformLocation);
    load("glUniformMatrix4fv", glUniformMatrix4fv);
    load("glUniform3f", glUniform3f);
    load("glUniform1i", glUniform1i);
    load("glGenerateMipmap", glGenerateMipmap);

    if (ok)
        Log::info("OpenGL functions loaded");
    return ok;
}
