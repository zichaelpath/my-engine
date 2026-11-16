#pragma once

#include "Mesh.hpp"

#include <GL/gl.h>

class Game;

// Viewport3D (Lesson 93) — the editor's 3D view: the current
// scene's heightfield, rendered by raw OpenGL into a
// framebuffer texture that an ImGui panel displays. The
// editor's window already owns a GL context (SFML made it
// one), so this class loads the engine's GL entry points on
// first use and renders into its own FBO alongside SFML's 2D
// drawing — the two halves of the engine, one window.
//
// The camera is a slow orbit: no input, just a turning look at
// the terrain from above, so the panel is alive while you work
// in the 2D view. The terrain mesh rebuilds lazily whenever
// the scene's tiles change (markDirty), through the same
// shared builder the standalone GL host uses.
class Viewport3D
{
public:
    void markDirty() { m_dirty = true; }

    // Render the scene into the viewport texture at (width,
    // height), advancing the orbit by dt seconds. Call with the
    // editor window's GL context active. Returns the GL texture
    // id to show, or 0 when the viewport is not ready (too
    // small, or GL failed to initialise).
    GLuint render(const Game& game, float dt, int width, int height);

private:
    bool init();
    void ensureTarget(int width, int height);
    void destroyTarget();

    bool m_triedInit = false;
    bool m_ready = false;
    bool m_dirty = true;
    float m_orbit = 0.f;
    GLuint m_program = 0;
    GLint m_mvpLoc = -1;
    GLint m_modelLoc = -1;
    GLint m_sunLoc = -1;
    GLint m_camLoc = -1;
    GLuint m_fbo = 0;
    GLuint m_colourTex = 0;
    GLuint m_depthRb = 0;
    int m_width = 0;
    int m_height = 0;
    Mesh m_terrain;
};
