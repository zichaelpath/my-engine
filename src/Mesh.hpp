#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Mesh (Lesson 91) — a VAO/VBO pair (plus an EBO when indexed),
// owned properly: constructed from plain vertex data and a
// layout, drawn with one call, deleted on destruction. Before
// this class, every mesh in the GL host was a loose pile of
// GLuint handles and attribute calls repeated per mesh; now the
// pile has a shape, and meshes are values the host can hold.
//
// The layout is a list of (location, component count) pairs in
// buffer order; the stride is their sum. The class is
// move-only: a VAO has exactly one owner, like a file handle.
//
// Construction needs a current GL context (the hosts construct
// meshes after theirs is active). Destruction guards the loader
// pointers, so a Mesh that never uploaded is safe to drop
// anywhere.
struct MeshAttrib
{
    unsigned int location;
    int components;
};

class Mesh
{
public:
    Mesh() = default;
    Mesh(const std::vector<float>& verts,
         const std::vector<MeshAttrib>& layout,
         const std::vector<std::uint32_t>& indices = {});
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;
    bool valid() const { return m_vao != 0; }
    int drawCount() const { return m_count; }

private:
    void destroy();

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
    int m_count = 0; // vertices, or indices when indexed
    bool m_indexed = false;
};

// OBJ loading (Lesson 91) — CPU-side only, no GL calls, so the
// headless harness tests it directly. parseObj is the worker;
// loadObj reads a file and hands its text over.
//
// Supported: v / vt / vn lines; faces in the v, v/vt, v//vn and
// v/vt/vn forms; polygons triangulated as a fan; negative
// (relative) indices. Vertices are deduplicated on their
// (v, vt, vn) triple. A face without normals gets a computed
// flat normal; a vertex without a UV gets (0, 0).
// Output layout: position (3) + normal (3) + uv (2), indexed.
bool parseObj(const std::string& text, std::vector<float>& outVerts,
              std::vector<std::uint32_t>& outIndices);
bool loadObj(const std::string& path, std::vector<float>& outVerts,
             std::vector<std::uint32_t>& outIndices);
