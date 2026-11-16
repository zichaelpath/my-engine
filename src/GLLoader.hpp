#pragma once

// GLLoader — the engine's own OpenGL function loader (Lesson 35).
//
// Why this exists: an OpenGL 3.3 core context only guarantees the
// OpenGL 1.1 entry points at link time. Everything newer — shaders,
// buffers, vertex arrays — must be asked of the driver at runtime,
// by name. Libraries like GLAD generate this file; an engine you
// intend to understand writes it, once, and watches it grow by one
// line per function the engine actually uses.
//
// GL 1.1 functions (glClear, glViewport, glGetString, ...) are
// declared by <GL/gl.h> and called directly. The post-1.1 functions
// below are pointers, filled in by loadGLFunctions() once a context
// is current. vendor/GL/glext.h is the official Khronos header: it
// supplies the PFN* types (and no prototypes, so the names are free
// for our pointers).

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

inline PFNGLCREATESHADERPROC glCreateShader = nullptr;
inline PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
inline PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
inline PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
inline PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
inline PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
inline PFNGLATTACHSHADERPROC glAttachShader = nullptr;
inline PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
inline PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
inline PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
inline PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
inline PFNGLDELETESHADERPROC glDeleteShader = nullptr;
inline PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
inline PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
inline PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
inline PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
inline PFNGLBUFFERDATAPROC glBufferData = nullptr;
inline PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
inline PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
inline PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
inline PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
inline PFNGLUNIFORM3FPROC glUniform3f = nullptr;
inline PFNGLUNIFORM1IPROC glUniform1i = nullptr;
inline PFNGLUNIFORM1FPROC glUniform1f = nullptr;
inline PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
inline PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
inline PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
inline PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
inline PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
inline PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
inline PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
inline PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
inline PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
inline PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
inline PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;
inline PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
inline PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
inline PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
// Note: glGenTextures/glBindTexture/glTexImage2D/glTexParameteri
// are NOT here — texture objects are OpenGL 1.1 (1997), so they are
// declared by <GL/gl.h> and called directly, like glClear.
// glActiveTexture (1.3) is likewise declared by this platform's
// <GL/gl.h> and called directly — it was added to the loader in
// Lesson 90 and removed the same day when the header objected.

// Fills every pointer above from the current context's driver.
// Returns false (and logs each miss) if any function is missing —
// call it once, right after the window's context is made current.
bool loadGLFunctions();
