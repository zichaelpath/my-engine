#pragma once

// On Windows, <GL/gl.h> is not self-contained: it spends the
// WINGDIAPI and APIENTRY macros that <windows.h> defines, so
// Windows must be included first (GLLoader.hpp carries the
// same preamble). NOMINMAX keeps the min/max macros out of
// every translation unit this header reaches. Found by the
// 1.8.0 Windows playtest build (Lesson 95) — Mesa's gl.h is
// self-contained, so the Linux rig never saw it.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <GL/gl.h>

// compileProgram (extracted in Lesson 93 from the GL host) —
// compile a vertex + fragment shader pair and link them into
// a program, logging every failure with its info log. Returns
// 0 on any failure. Shared by every GL host and viewport: the
// standalone MyGL and the editor's 3D view compile their
// shaders through this one door.
GLuint compileProgram(const char* vertexSource,
                      const char* fragmentSource);
