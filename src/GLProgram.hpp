#pragma once

#include <GL/gl.h>

// compileProgram (extracted in Lesson 93 from the GL host) —
// compile a vertex + fragment shader pair and link them into
// a program, logging every failure with its info log. Returns
// 0 on any failure. Shared by every GL host and viewport: the
// standalone MyGL and the editor's 3D view compile their
// shaders through this one door.
GLuint compileProgram(const char* vertexSource,
                      const char* fragmentSource);
