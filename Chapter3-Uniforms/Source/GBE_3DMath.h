//
//  GBEMath.h
//  Chapter3-Uniforms
//
//  Created by Jonathan Fischer on 3/13/25.
//
//  Vector and matrix math data structures and functions.

#ifndef GpuByExample_Math_h
#define GpuByExample_Math_h

typedef struct GBE_Vector3 {
    float x, y, z;
} GBE_Vector3;

typedef struct GBE_Vector4 {
    float x, y, z, w;
} GBE_Vector4;

typedef struct GBE_Matrix4x4 {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} GBE_Matrix4x4;

extern const GBE_Vector3   kZeroVector3;
extern const GBE_Matrix4x4 kIdentityMatrix;

GBE_Vector3 GBE_Vector3Add(GBE_Vector3 a, GBE_Vector3 b);
GBE_Vector3 GBE_Vector3Subtract(GBE_Vector3 a, GBE_Vector3 b);
GBE_Vector3 GBE_Vector3Negate(GBE_Vector3 v);
GBE_Vector3 GBE_Vector3Scale(GBE_Vector3 v, float scalar);
float       GBE_Vector3Length(GBE_Vector3 v);
GBE_Vector3 GBE_Vector3Normal(GBE_Vector3 v);
float       GBE_Vector3Distance(GBE_Vector3 a, GBE_Vector3 b);
float       GBE_Vector3DotProduct(GBE_Vector3 a, GBE_Vector3 b);
GBE_Vector3 GBE_Vector3CrossProduct(GBE_Vector3 a, GBE_Vector3 b);

GBE_Vector3 GBE_Matrix4x4TransformVector3(GBE_Vector3 v, GBE_Matrix4x4 m);
GBE_Vector4 GBE_Matrix4x4TransformVector4(GBE_Vector4 v, GBE_Matrix4x4 m);

GBE_Matrix4x4 GBE_Matrix4x4Multiply(GBE_Matrix4x4 a, GBE_Matrix4x4 b);
GBE_Matrix4x4 GBE_Matrix4x4Transpose(GBE_Matrix4x4 m);
GBE_Matrix4x4 GBE_Matrix4x4Translation(GBE_Vector3 t);
GBE_Matrix4x4 GBE_Matrix4x4UniformScale(float scalar);
GBE_Matrix4x4 GBE_Matrix4x4RotateAxisAngle(GBE_Vector3 axis, float angleInRadians);
GBE_Matrix4x4 GBE_Matrix4x4Perspective(float aspect, float fovy, float near, float far);

#endif /* GpuByExample_Math_h */

