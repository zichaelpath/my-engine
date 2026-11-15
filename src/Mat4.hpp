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

    const float* data() const { return m; }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b)
{
    return Mat4::multiply(a, b);
}
