//
//  GBEMath.c
//  Chapter3-Uniforms
//
//  Created by Jonathan Fischer on 3/13/25.
//
//  Vector and matrix math data structures and functions.

#include <math.h>
#include "GBE_3DMath.h"

const GBE_Vector3 kZeroVector3 = { 0, 0, 0 };

const GBE_Matrix4x4 kIdentityMatrix = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

#pragma mark GBE_Vector3

GBE_Vector3 GBE_Vector3Add(GBE_Vector3 a, GBE_Vector3 b)
{
    return (GBE_Vector3) {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

GBE_Vector3 GBE_Vector3Subtract(GBE_Vector3 a, GBE_Vector3 b)
{
    return (GBE_Vector3) {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

GBE_Vector3 GBE_Vector3Negate(GBE_Vector3 v)
{
    return (GBE_Vector3) { -v.x, -v.y, -v.z };
}

GBE_Vector3 GBE_Vector3Scale(GBE_Vector3 a, float scalar)
{
    return (GBE_Vector3) {
        a.x * scalar,
        a.y * scalar,
        a.z * scalar
    };
}

float GBE_Vector3Length(GBE_Vector3 v)
{
    float magnitudeSquared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (magnitudeSquared == 0.0f) {
        return 0.0f;
    }

    return sqrtf(magnitudeSquared);
}

GBE_Vector3 GBE_Vector3Normal(GBE_Vector3 v)
{
    float magnitude = GBE_Vector3Length(v);
    if (magnitude == 0.0) {
        return kZeroVector3;
    }

    return GBE_Vector3Scale(v, 1.0f / magnitude);
}

float GBE_Vector3Distance(GBE_Vector3 a, GBE_Vector3 b)
{
    return GBE_Vector3Length(GBE_Vector3Subtract(a, b));
}

float GBE_Vector3DotProduct(GBE_Vector3 a, GBE_Vector3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

GBE_Vector3 GBE_Vector3CrossProduct(GBE_Vector3 a, GBE_Vector3 b)
{
    return (GBE_Vector3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

#pragma mark - GBE_Matrix4x4

GBE_Vector3 GBE_Matrix4x4TransformVector3(GBE_Vector3 v, GBE_Matrix4x4 m)
{
    GBE_Vector3 r;
    r.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30;
    r.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31;
    r.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32;
    return r;
}

GBE_Vector4 GBE_Matrix4x4TransformVector4(GBE_Vector4 v, GBE_Matrix4x4 m)
{
    GBE_Vector4 r;
    r.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30;
    r.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31;
    r.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32;
    r.w = v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33;
    return r;
}

GBE_Matrix4x4 GBE_Matrix4x4Multiply(GBE_Matrix4x4 a, GBE_Matrix4x4 b)
{
    GBE_Matrix4x4 m;
    m.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30;
    m.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31;
    m.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32;
    m.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33;

    m.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30;
    m.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
    m.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
    m.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;

    m.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30;
    m.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
    m.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
    m.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;

    m.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30;
    m.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;
    m.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;
    m.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;

    return m;
}

GBE_Matrix4x4 GBE_Matrix4x4Translation(GBE_Vector3 t)
{
    return (GBE_Matrix4x4) {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1,
        t.x, t.y, t.z, 1
    };
}

GBE_Matrix4x4 GBE_Matrix4x4UniformScale(float s)
{
    return (GBE_Matrix4x4) {
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        0, 0, 0, 1
    };
}

GBE_Matrix4x4 GBE_Matrix4x4RotateAxisAngle(GBE_Vector3 axis, float angleInRadians)
{
    float c = cos(angleInRadians);
    float s = sin(angleInRadians);

    GBE_Vector3 X;
    X.x = axis.x * axis.x + (1 - axis.x * axis.x) * c;
    X.y = axis.x * axis.y * (1 - c) - axis.z * s;
    X.z = axis.x * axis.z * (1 - c) + axis.y * s;

    GBE_Vector3 Y;
    Y.x = axis.x * axis.y * (1 - c) + axis.z * s;
    Y.y = axis.y * axis.y + (1 - axis.y * axis.y) * c;
    Y.z = axis.y * axis.z * (1 - c) - axis.x * s;

    GBE_Vector3 Z;
    Z.x = axis.x * axis.z * (1 - c) - axis.y * s;
    Z.y = axis.y * axis.z * (1 - c) + axis.x * s;
    Z.z = axis.z * axis.z + (1 - axis.z * axis.z) * c;

    return (GBE_Matrix4x4) {
        X.x, X.y, X.z, 0,
        Y.x, Y.y, Y.z, 0,
        Z.x, Z.y, Z.z, 0,
        0, 0, 0, 1
    };
}

GBE_Matrix4x4 GBE_Matrix4x4Perspective(float aspect, float fovy, float near, float far)
{
    float yScale = 1 / tan(fovy * 0.5);
    float xScale = yScale / aspect;
    float zRange = far - near;
    float zScale = -(far + near) / zRange;
    float wzScale = -2 * far * near / zRange;

    return (GBE_Matrix4x4) {
        xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, zScale, -1,
        0, 0, wzScale, 0
    };
}

GBE_Matrix4x4 GBE_Matrix4x4Transpose(GBE_Matrix4x4 m)
{
    return (GBE_Matrix4x4) {
        m.m00, m.m10, m.m20, m.m30,
        m.m01, m.m11, m.m21, m.m31,
        m.m02, m.m12, m.m22, m.m32,
        m.m03, m.m13, m.m23, m.m33
    };
}
