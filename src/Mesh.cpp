#include "Mesh.hpp"

#include "GLLoader.hpp"
#include "Log.hpp"

#include <array>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <tuple>

Mesh::Mesh(const std::vector<float>& verts,
           const std::vector<MeshAttrib>& layout,
           const std::vector<std::uint32_t>& indices)
{
    if (verts.empty() || layout.empty())
        return;
    int strideFloats = 0;
    for (const MeshAttrib& a : layout)
        strideFloats += a.components;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);
    if (!indices.empty())
    {
        glGenBuffers(1, &m_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() *
                                    sizeof(std::uint32_t)),
            indices.data(), GL_STATIC_DRAW);
        m_indexed = true;
        m_count = static_cast<int>(indices.size());
    }
    else
    {
        m_count = static_cast<int>(verts.size() / strideFloats);
    }
    std::size_t offset = 0;
    for (const MeshAttrib& a : layout)
    {
        glVertexAttribPointer(
            a.location, a.components, GL_FLOAT, GL_FALSE,
            static_cast<GLsizei>(strideFloats * sizeof(float)),
            reinterpret_cast<const void*>(offset * sizeof(float)));
        glEnableVertexAttribArray(a.location);
        offset += static_cast<std::size_t>(a.components);
    }
    glBindVertexArray(0);
}

Mesh::~Mesh() { destroy(); }

Mesh::Mesh(Mesh&& other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo),
      m_count(other.m_count), m_indexed(other.m_indexed)
{
    other.m_vao = other.m_vbo = other.m_ebo = 0;
    other.m_count = 0;
    other.m_indexed = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        destroy();
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ebo = other.m_ebo;
        m_count = other.m_count;
        m_indexed = other.m_indexed;
        other.m_vao = other.m_vbo = other.m_ebo = 0;
        other.m_count = 0;
        other.m_indexed = false;
    }
    return *this;
}

void Mesh::destroy()
{
    // The loader pointers are null when no context ever loaded
    // them (a Mesh dropped by a headless tool) — deleting then
    // would be a call through a null pointer, so guard first.
    if (glDeleteBuffers == nullptr || glDeleteVertexArrays == nullptr)
        return;
    if (m_ebo != 0)
        glDeleteBuffers(1, &m_ebo);
    if (m_vbo != 0)
        glDeleteBuffers(1, &m_vbo);
    if (m_vao != 0)
        glDeleteVertexArrays(1, &m_vao);
    m_vao = m_vbo = m_ebo = 0;
}

void Mesh::draw() const
{
    if (m_vao == 0)
        return;
    glBindVertexArray(m_vao);
    if (m_indexed)
        glDrawElements(GL_TRIANGLES, m_count, GL_UNSIGNED_INT,
                       nullptr);
    else
        glDrawArrays(GL_TRIANGLES, 0, m_count);
}

namespace
{

// One OBJ index, resolved: OBJ counts from 1, and a negative
// index counts back from the most recent (-1 = last). -1 in the
// result means "absent" (a vt or vn the face didn't give).
int resolveIndex(int raw, std::size_t count)
{
    if (raw > 0)
        return raw - 1;
    if (raw < 0)
        return static_cast<int>(count) + raw;
    return -1;
}

} // namespace

bool parseObj(const std::string& text, std::vector<float>& outVerts,
              std::vector<std::uint32_t>& outIndices)
{
    outVerts.clear();
    outIndices.clear();
    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 2>> uvs;
    std::vector<std::array<float, 3>> normals;
    std::map<std::tuple<int, int, int>, std::uint32_t> dedupe;

    auto emitVertex = [&](int v, int vt, int vn) -> std::uint32_t {
        const auto key = std::make_tuple(v, vt, vn);
        if (auto it = dedupe.find(key); it != dedupe.end())
            return it->second;
        const auto& p = positions[static_cast<std::size_t>(v)];
        const auto& n = normals[static_cast<std::size_t>(vn)];
        const std::array<float, 2> t =
            vt >= 0 ? uvs[static_cast<std::size_t>(vt)]
                    : std::array<float, 2>{0.f, 0.f};
        outVerts.insert(outVerts.end(),
                        {p[0], p[1], p[2], n[0], n[1], n[2], t[0],
                         t[1]});
        const auto index =
            static_cast<std::uint32_t>(outVerts.size() / 8 - 1);
        dedupe[key] = index;
        return index;
    };

    std::istringstream stream(text);
    std::string line;
    int lineNo = 0;
    while (std::getline(stream, line))
    {
        ++lineNo;
        std::istringstream ls(line);
        std::string tag;
        if (!(ls >> tag))
            continue;
        if (tag == "v")
        {
            std::array<float, 3> p{};
            if (!(ls >> p[0] >> p[1] >> p[2]))
                return false;
            positions.push_back(p);
        }
        else if (tag == "vt")
        {
            std::array<float, 2> t{};
            if (!(ls >> t[0] >> t[1]))
                return false;
            uvs.push_back(t);
        }
        else if (tag == "vn")
        {
            std::array<float, 3> n{};
            if (!(ls >> n[0] >> n[1] >> n[2]))
                return false;
            normals.push_back(n);
        }
        else if (tag == "f")
        {
            // Face vertices as resolved (v, vt, vn) triples.
            std::vector<std::tuple<int, int, int>> face;
            std::string token;
            bool bad = false;
            while (ls >> token)
            {
                int rv = 0, rvt = 0, rvn = 0;
                bool hasVt = false, hasVn = false;
                // Split on '/': v, then optionally vt, then vn
                // (v//vn leaves the middle empty).
                std::string parts[3];
                int part = 0;
                for (char c : token)
                {
                    if (c == '/')
                    {
                        if (++part > 2)
                        {
                            bad = true;
                            break;
                        }
                    }
                    else
                        parts[part] += c;
                }
                if (bad)
                    break;
                try
                {
                    rv = std::stoi(parts[0]);
                    if (part >= 1 && !parts[1].empty())
                    {
                        rvt = std::stoi(parts[1]);
                        hasVt = true;
                    }
                    if (part >= 2 && !parts[2].empty())
                    {
                        rvn = std::stoi(parts[2]);
                        hasVn = true;
                    }
                }
                catch (...)
                {
                    bad = true;
                    break;
                }
                const int v = resolveIndex(rv, positions.size());
                const int vt =
                    hasVt ? resolveIndex(rvt, uvs.size()) : -1;
                const int vn =
                    hasVn ? resolveIndex(rvn, normals.size()) : -1;
                if (v < 0 ||
                    v >= static_cast<int>(positions.size()) ||
                    (hasVt && (vt < 0 ||
                               vt >= static_cast<int>(uvs.size()))) ||
                    (hasVn && (vn < 0 ||
                               vn >= static_cast<int>(
                                        normals.size()))))
                {
                    bad = true;
                    break;
                }
                face.emplace_back(v, vt, vn);
            }
            if (bad || face.size() < 3)
            {
                Log::error("OBJ: bad face on line " +
                           std::to_string(lineNo));
                return false;
            }
            // A face without normals gets one computed flat
            // normal, appended and shared by the whole face.
            if (std::get<2>(face[0]) < 0)
            {
                const auto& a =
                    positions[static_cast<std::size_t>(
                        std::get<0>(face[0]))];
                const auto& b =
                    positions[static_cast<std::size_t>(
                        std::get<0>(face[1]))];
                const auto& c =
                    positions[static_cast<std::size_t>(
                        std::get<0>(face[2]))];
                const float ux = b[0] - a[0], uy = b[1] - a[1],
                            uz = b[2] - a[2];
                const float vx = c[0] - a[0], vy = c[1] - a[1],
                            vz = c[2] - a[2];
                float nx = uy * vz - uz * vy;
                float ny = uz * vx - ux * vz;
                float nz = ux * vy - uy * vx;
                const float len =
                    std::sqrt(nx * nx + ny * ny + nz * nz);
                if (len > 0.f)
                {
                    nx /= len;
                    ny /= len;
                    nz /= len;
                }
                normals.push_back({nx, ny, nz});
                const int flat =
                    static_cast<int>(normals.size()) - 1;
                for (auto& vertex : face)
                    std::get<2>(vertex) = flat;
            }
            // Fan triangulation: (0, i, i+1).
            for (std::size_t i = 1; i + 1 < face.size(); ++i)
            {
                for (int k : {0, static_cast<int>(i),
                              static_cast<int>(i) + 1})
                {
                    const auto& [v, vt, vn] =
                        face[static_cast<std::size_t>(k)];
                    outIndices.push_back(emitVertex(v, vt, vn));
                }
            }
        }
        // Everything else (o, g, s, mtllib, usemtl, comments) is
        // noted and passed over: this loader models geometry,
        // not scenes.
    }
    if (outIndices.empty())
    {
        Log::error("OBJ: no faces found");
        return false;
    }
    return true;
}

bool loadObj(const std::string& path, std::vector<float>& outVerts,
             std::vector<std::uint32_t>& outIndices)
{
    std::ifstream in(path);
    if (!in)
    {
        Log::error("OBJ: cannot open " + path);
        return false;
    }
    std::ostringstream text;
    text << in.rdbuf();
    if (!parseObj(text.str(), outVerts, outIndices))
    {
        Log::error("OBJ: failed to parse " + path);
        return false;
    }
    Log::info("OBJ loaded: " + path + " (" +
              std::to_string(outVerts.size() / 8) + " vertices, " +
              std::to_string(outIndices.size() / 3) +
              " triangles)");
    return true;
}
