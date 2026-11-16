#pragma once

// Mat4 — 4x4 matrices for the 3D path (Lesson 36), header-only in
// the same style as Math.hpp. Column-major storage (m[col*4 + row]),
// which is the layout OpenGL expects: the array can be handed to
// glUniformMatrix4fv with transpose = GL_FALSE, untouched.
//
// Conventions (OpenGL's): right-handed coordinates, the camera looks
// down -Z, and transforms compose left-to-right — in
// projection * view * model, the model matrix touches the vertex
// first. Multiplication is written multiply(a, b) = a * b, "apply b,
// then a", matching that reading.

#include <cmath>

// Vec3 — the 3D companion to Math.hpp's 2D helpers (Lesson 37).
struct Vec3
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3& operator+=(const Vec3& o)
    {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
};

inline float dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline Vec3 normalized(const Vec3& v)
{
    float len = std::sqrt(dot(v, v));
    if (len <= 0.f)
        return v;
    return v * (1.f / len);
}

struct Mat4
{
    float m[16] = {};

    static Mat4 identity()
    {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f;
        return r;
    }

    static Mat4 multiply(const Mat4& a, const Mat4& b)
    {
        Mat4 r;
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
            {
                float sum = 0.f;
                for (int k = 0; k < 4; ++k)
                    sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                r.m[col * 4 + row] = sum;
            }
        return r;
    }

    static Mat4 translation(float x, float y, float z)
    {
        Mat4 r = identity();
        r.m[12] = x;
        r.m[13] = y;
        r.m[14] = z;
        return r;
    }

    static Mat4 scaling(float x, float y, float z)
    {
        Mat4 r = identity();
        r.m[0] = x;
        r.m[5] = y;
        r.m[10] = z;
        return r;
    }

    static Mat4 rotationX(float radians)
    {
        Mat4 r = identity();
        float c = std::cos(radians);
        float s = std::sin(radians);
        r.m[5] = c;
        r.m[6] = s;
        r.m[9] = -s;
        r.m[10] = c;
        return r;
    }

    static Mat4 rotationY(float radians)
    {
        Mat4 r = identity();
        float c = std::cos(radians);
        float s = std::sin(radians);
        r.m[0] = c;
        r.m[2] = -s;
        r.m[8] = s;
        r.m[10] = c;
        return r;
    }

    static Mat4 rotationZ(float radians)
    {
        Mat4 r = identity();
        float c = std::cos(radians);
        float s = std::sin(radians);
        r.m[0] = c;
        r.m[1] = s;
        r.m[4] = -s;
        r.m[5] = c;
        return r;
    }

    // Perspective projection: fovY in radians, depth in (near, far)
    // maps to NDC z in (-1, 1), with -Z forward.
    static Mat4 perspective(float fovYRadians, float aspect, float nearPlane, float farPlane)
    {
        Mat4 r;
        float t = 1.f / std::tan(fovYRadians / 2.f);
        r.m[0] = t / aspect;
        r.m[5] = t;
        r.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
        r.m[11] = -1.f;
        r.m[14] = 2.f * farPlane * nearPlane / (nearPlane - farPlane);
        return r;
    }

    // Orthographic projection (Lesson 90): the box [l,r]x[b,t]x
    // [n,f] maps linearly onto the NDC cube — no perspective
    // divide, no foreshortening. A shadow camera wants exactly
    // this: the sun's rays are parallel, so its "lens" is a box.
    // View space looks down -Z, so depths are -z in [n, f].
    static Mat4 orthographic(float left, float right, float bottom,
                             float top, float nearPlane, float farPlane)
    {
        Mat4 r;
        r.m[0] = 2.f / (right - left);
        r.m[5] = 2.f / (top - bottom);
        r.m[10] = -2.f / (farPlane - nearPlane);
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[15] = 1.f;
        return r;
    }

    // The view matrix for a camera at eye looking at center: builds
    // an orthonormal basis (side, up, -forward) from the two points
    // and a world-up hint, then inverts the camera's placement —
    // a view matrix is the camera transform turned inside out.
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
    {
        Vec3 f = normalized(center - eye);
        Vec3 s = normalized(cross(f, up));
        Vec3 u = cross(s, f);
        Mat4 r = identity();
        r.m[0] = s.x;
        r.m[1] = u.x;
        r.m[2] = -f.x;
        r.m[4] = s.y;
        r.m[5] = u.y;
        r.m[6] = -f.y;
        r.m[8] = s.z;
        r.m[9] = u.z;
        r.m[10] = -f.z;
        r.m[12] = -dot(s, eye);
        r.m[13] = -dot(u, eye);
        r.m[14] = dot(f, eye);
        return r;
    }

    const float* data() const { return m; }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b)
{
    return Mat4::multiply(a, b);
}
