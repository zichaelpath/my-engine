#include "Viewport3D.hpp"

#include "GLLoader.hpp"
#include "GLProgram.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include "Mat4.hpp"
#include "TerrainMesh.hpp"

#include <cmath>

namespace
{

// The viewport's own lit-terrain program: the same vertex
// layout and Lambert + Blinn-Phong lighting as the standalone
// host's terrain program, minus shadows and fog (the panel is
// a workbench view, not the game). One day both hosts will
// share a shader library; today the honest scope is a copy
// with a comment saying so.
const char* kVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aSpec;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vColor;
out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vSpec;
void main()
{
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vColor = aColor;
    vNormal = mat3(uModel) * aNormal;
    vSpec = aSpec;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char* kFragmentShader = R"(
#version 330 core
in vec3 vColor;
in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vSpec;
uniform vec3 uSunDir;
uniform vec3 uCameraPos;
out vec4 FragColor;
void main()
{
    vec3 n = normalize(vNormal);
    float diffuse = max(dot(n, uSunDir), 0.0);
    float light = 0.35 + 0.65 * diffuse;
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 halfVec = normalize(uSunDir + viewDir);
    float spec = pow(max(dot(n, halfVec), 0.0), vSpec.x) * vSpec.y;
    FragColor = vec4(vColor * light + spec, 1.0);
}
)";

} // namespace

bool Viewport3D::init()
{
    m_triedInit = true;
    if (!loadGLFunctions())
    {
        Log::error("3D viewport: GL entry points failed to load");
        return false;
    }
    m_program = compileProgram(kVertexShader, kFragmentShader);
    if (m_program == 0)
        return false;
    m_mvpLoc = glGetUniformLocation(m_program, "uMVP");
    m_modelLoc = glGetUniformLocation(m_program, "uModel");
    m_sunLoc = glGetUniformLocation(m_program, "uSunDir");
    m_camLoc = glGetUniformLocation(m_program, "uCameraPos");
    if (m_mvpLoc < 0 || m_modelLoc < 0 || m_sunLoc < 0 ||
        m_camLoc < 0)
    {
        Log::error("3D viewport: a required uniform was not found");
        return false;
    }
    Log::info("3D viewport ready");
    return true;
}

void Viewport3D::destroyTarget()
{
    if (glDeleteFramebuffers != nullptr && m_fbo != 0)
        glDeleteFramebuffers(1, &m_fbo);
    if (glDeleteRenderbuffers != nullptr && m_depthRb != 0)
        glDeleteRenderbuffers(1, &m_depthRb);
    if (m_colourTex != 0)
        glDeleteTextures(1, &m_colourTex);
    m_fbo = m_depthRb = m_colourTex = 0;
    m_width = m_height = 0;
}

void Viewport3D::ensureTarget(int width, int height)
{
    if (m_fbo != 0 && width == m_width && height == m_height)
        return;
    destroyTarget();
    glGenTextures(1, &m_colourTex);
    glBindTexture(GL_TEXTURE_2D, m_colourTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_colourTex, 0);
    glGenRenderbuffers(1, &m_depthRb);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRb);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE)
    {
        Log::error("3D viewport: framebuffer is not complete");
        destroyTarget();
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_width = width;
    m_height = height;
}

GLuint Viewport3D::render(const Game& game, float dt, int width,
                          int height)
{
    if (width < 64 || height < 64)
        return 0;
    if (!m_triedInit)
        m_ready = init();
    if (!m_ready)
        return 0;
    ensureTarget(width, height);
    if (m_fbo == 0)
        return 0;
    if (m_dirty)
    {
        m_terrain = Mesh(buildTerrain(game),
                         {{0, 3}, {1, 3}, {2, 3}, {3, 2}});
        m_dirty = false;
    }
    m_orbit += dt * 0.12f;

    // The orbit: above the world's centre, circling slowly.
    const float radius = 52.f;
    const Vec3 eye{std::cos(m_orbit) * radius, 34.f,
                   std::sin(m_orbit) * radius};
    const Mat4 view = Mat4::lookAt(eye, Vec3{0.f, 0.f, 0.f},
                                   Vec3{0.f, 1.f, 0.f});
    const Mat4 projection = Mat4::perspective(
        55.f * 3.14159265f / 180.f,
        static_cast<float>(width) / static_cast<float>(height),
        0.1f, 300.f);
    const Vec3 sunDir = normalized(Vec3{-0.55f, 0.75f, 0.35f});

    // SFML state hygiene: the editor's 2D drawing shares this
    // context. Save the pieces the render touches and hand them
    // back — depth testing and the viewport are the big ones,
    // and SFML resets its own state on its next draw anyway.
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.74f, 0.82f, 0.92f, 1.0f); // the horizon colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_program);
    const Mat4 mvp = projection * view;
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, mvp.data());
    glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE,
                       Mat4::identity().data());
    glUniform3f(m_sunLoc, sunDir.x, sunDir.y, sunDir.z);
    glUniform3f(m_camLoc, eye.x, eye.y, eye.z);
    m_terrain.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glViewport(prevViewport[0], prevViewport[1], prevViewport[2],
               prevViewport[3]);
    if (prevDepth == GL_FALSE)
        glDisable(GL_DEPTH_TEST);
    return m_colourTex;
}
