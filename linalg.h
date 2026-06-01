//
// linalg.h
//
// This is a public domain C/C++ library.
// No warranty implied, use at your own risk!
//
// This library is primarily focused on graphics programming.
// There are many libraries out there that are more optimized
// and feature-complete than this one. I made this library mainly
// for my own personal needs and preferred ergonomics.
//
// I believe most, if not all, of the functionality provided here
// can also be found in other popular math libraries. There is
// nothing particularly unique about it. The one thing that is
// probably worth explaining specifically is the operator overloading.
//
// I tried to keep the operators relatively close to actual math
// notation. Because of that, some operations that are allowed in
// shader languages such as GLSL are intentionally disallowed here.
//
// For example, given a vector v, GLSL allows operations like:
//
//   v + 1.0
//   1.0 + v
//   1.0 - v
//   1.0 / v
//
// These operations are interpreted component-wise in GLSL, but
// they do not really correspond to standard linear algebra notation.
//
// In this library, I intentionally restrict certain operations
// while still allowing some practical conveniences commonly used
// in graphics programming.
//
// In summary, these are the rules:
//
//   v + u    allowed
//   v + s    forbidden
//   v * u    component-wise
//
//   m1 + m2  allowed
//   m1 + s   forbidden
//   m1 * m2  matrix multiplication
//

#ifndef LINALG_H
#define LINALG_H

typedef struct { float x, y;       } Vec2;
typedef struct { float x, y, z;    } Vec3;
typedef struct { float x, y, z, w; } Vec4;
typedef struct { float m[16];      } Mat4;
typedef struct { float x, y, z, w; } Quat;
typedef struct {
    Vec3 position;
    Quat rotation;
    Vec3 scale;
} Transform;

// Column-major memory layout
enum {
    MAT4_00, MAT4_10, MAT4_20, MAT4_30,
    MAT4_01, MAT4_11, MAT4_21, MAT4_31,
    MAT4_02, MAT4_12, MAT4_22, MAT4_32,
    MAT4_03, MAT4_13, MAT4_23, MAT4_33,
};

#define LINALG_EPSILON 1e-5

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

Vec2 vec2_make(float x, float y);
bool vec2_eq(Vec2 a, Vec2 b);
Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_neg(Vec2 v);
Vec2 vec2_mul(Vec2 a, Vec2 b);
Vec2 vec2_div(Vec2 a, Vec2 b);
Vec2 vec2_scale(Vec2 v, float s);
float vec2_dot(Vec2 a, Vec2 b);
float vec2_length(Vec2 v);
float vec2_length_sq(Vec2 v);
Vec2 vec2_norm(Vec2 v);

Vec3 vec3_make(float x, float y, float z);
bool vec3_eq(Vec3 a, Vec3 b);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_neg(Vec3 v);
Vec3 vec3_mul(Vec3 a, Vec3 b);
Vec3 vec3_div(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, float s);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_length(Vec3 v);
float vec3_length_sq(Vec3 v);
Vec3 vec3_norm(Vec3 v);
Vec3 vec3_rotate_quat(Vec3 v, Quat q);

Vec4 vec4_make(float x, float y, float z, float w);
bool vec4_eq(Vec4 a, Vec4 b);
Vec4 vec4_add(Vec4 a, Vec4 b);
Vec4 vec4_sub(Vec4 a, Vec4 b);
Vec4 vec4_neg(Vec4 v);
Vec4 vec4_mul(Vec4 a, Vec4 b);
Vec4 vec4_div(Vec4 a, Vec4 b);
Vec4 vec4_scale(Vec4 v, float s);
float vec4_dot(Vec4 a, Vec4 b);
float vec4_length(Vec4 v);
float vec4_length_sq(Vec4 v);
Vec4 vec4_norm(Vec4 v);

// These are column-major, right-handed, column vectors,
// i.e. result = M * v, and transforms compose as M = T * R * S
Mat4 mat4_identity(void);
bool mat4_eq(Mat4 a, Mat4 b);
Mat4 mat4_add(Mat4 a, Mat4 b);
Mat4 mat4_sub(Mat4 a, Mat4 b);
Mat4 mat4_neg(Mat4 m);
Mat4 mat4_mul(Mat4 a, Mat4 b);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);
Mat4 mat4_mul_scalar(Mat4 m, float s);
Mat4 mat4_translate(Vec3 t);
Mat4 mat4_scaling(Vec3 s);
Mat4 mat4_rotate(float rads, Vec3 v);
Mat4 mat4_from_quat(Quat q);
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);
Mat4 mat4_perspective(float fov_rads, float aspect, float near, float far);
Mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far);
Mat4 mat4_transpose(Mat4 m);
Mat4 mat4_inverse(Mat4 m);

// Quaternion multiplication order matters. This implementation uses Hamilton product,
// where quat_mul(a, b) means apply b then a (same convention as mat4_mul).
Quat quat_make(float x, float y, float z, float w);
Quat quat_identity(void);
bool quat_eq(Quat a, Quat b);
Quat quat_add(Quat a, Quat b);
Quat quat_sub(Quat a, Quat b);
Quat quat_neg(Quat q);
Quat quat_mul(Quat a, Quat b);
Quat quat_scale(Quat q, float s);
Quat quat_norm(Quat q);
Quat quat_from_axis_angle(Vec3 axis, float rads);
Quat quat_from_euler(float pitch, float yaw, float roll);
Mat4 quat_to_mat4(Quat q);
Quat quat_slerp(Quat a, Quat b, float t);

// transform_mul(a, b) should mean apply b first, then a, matching mat4_mul and quat_mul.
Transform transform_make(Vec3 pos, Quat rot, Vec3 scale);
Mat4 transform_to_mat4(Transform t);
Transform transform_mul(Transform a, Transform b);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

// Basic operations
bool operator == (Vec2 a, Vec2 b);
bool operator != (Vec2 a, Vec2 b);
Vec2 operator + (Vec2 a, Vec2 b);
Vec2 operator - (Vec2 a, Vec2 b);
Vec2 operator - (Vec2 v); // negation 
Vec2 operator * (Vec2 a, Vec2 b); // component-wise
Vec2 operator / (Vec2 a, Vec2 b); // component-wise
// Scalar operations
Vec2 operator * (Vec2 v, float s);
Vec2 operator * (float s, Vec2 v);
Vec2 operator / (Vec2 v, float s);
// Compound assignment
Vec2 &operator += (Vec2 &lhs, Vec2 rhs);
Vec2 &operator -= (Vec2 &lhs, Vec2 rhs);
Vec2 &operator *= (Vec2 &lhs, Vec2 rhs);
Vec2 &operator *= (Vec2 &lhs, float rhs);
Vec2 &operator /= (Vec2 &lhs, Vec2 rhs);
Vec2 &operator /= (Vec2 &lhs, float rhs);

// Basic operations
bool operator == (Vec3 a, Vec3 b);
bool operator != (Vec3 a, Vec3 b);
Vec3 operator + (Vec3 a, Vec3 b);
Vec3 operator - (Vec3 a, Vec3 b);
Vec3 operator - (Vec3 v); // negation
Vec3 operator * (Vec3 a, Vec3 b); // component-wise
Vec3 operator / (Vec3 a, Vec3 b); // component-wise
// Scalar operations
Vec3 operator * (Vec3 v, float s);
Vec3 operator * (float s, Vec3 v);
Vec3 operator / (Vec3 v, float s);
// Compound assignment
Vec3 &operator += (Vec3 &lhs, Vec3 rhs);
Vec3 &operator -= (Vec3 &lhs, Vec3 rhs);
Vec3 &operator *= (Vec3 &lhs, Vec3 rhs);
Vec3 &operator *= (Vec3 &lhs, float rhs);
Vec3 &operator /= (Vec3 &lhs, Vec3 rhs);
Vec3 &operator /= (Vec3 &lhs, float rhs);

// Basic operations
bool operator == (Vec4 a, Vec4 b);
bool operator != (Vec4 a, Vec4 b);
Vec4 operator + (Vec4 a, Vec4 b);
Vec4 operator - (Vec4 a, Vec4 b);
Vec4 operator - (Vec4 v); // negation
Vec4 operator * (Vec4 a, Vec4 b); // component-wise
Vec4 operator / (Vec4 a, Vec4 b); // component-wise
// Scalar operations
Vec4 operator * (Vec4 v, float s);
Vec4 operator * (float s, Vec4 v);
Vec4 operator / (Vec4 v, float s);
// Compound assignment
Vec4 &operator += (Vec4 &lhs, Vec4 rhs);
Vec4 &operator -= (Vec4 &lhs, Vec4 rhs);
Vec4 &operator *= (Vec4 &lhs, Vec4 rhs);
Vec4 &operator *= (Vec4 &lhs, float rhs);
Vec4 &operator /= (Vec4 &lhs, Vec4 rhs);
Vec4 &operator /= (Vec4 &lhs, float rhs);

// Basic operations
bool operator == (Mat4 a, Mat4 b);
bool operator != (Mat4 a, Mat4 b);
Mat4 operator + (Mat4 a, Mat4 b);
Mat4 operator - (Mat4 a, Mat4 b);
Mat4 operator - (Mat4 m); // negation
Mat4 operator * (Mat4 a, Mat4 b); // matrix multiplication
Vec4 operator * (Mat4 m, Vec4 v); // matrix multiplication
// Scalar operations
Mat4 operator * (Mat4 m, float s);
Mat4 operator * (float s, Mat4 m);
Mat4 operator / (Mat4 m, float s);
// Compound assignment
Mat4 &operator += (Mat4 &lhs, Mat4 rhs);
Mat4 &operator -= (Mat4 &lhs, Mat4 rhs);
Mat4 &operator *= (Mat4 &lhs, Mat4 rhs);
Mat4 &operator *= (Mat4 &lhs, float rhs);
Mat4 &operator /= (Mat4 &lhs, float rhs);

// Basic operations
bool operator == (Quat a, Quat b);
bool operator != (Quat a, Quat b);
Quat operator + (Quat a, Quat b);
Quat operator - (Quat a, Quat b);
Quat operator - (Quat q); // negation
Quat operator * (Quat a, Quat b); // component-wise
Vec3 operator * (Quat q, Vec3 v); // component-wise
// Scalar operations
Quat operator * (Quat q, float s);
Quat operator * (float s, Quat q);
Quat operator / (Quat q, float s);
// Compound assignment
Quat &operator += (Quat &lhs, Quat rhs);
Quat &operator -= (Quat &lhs, Quat rhs);
Quat &operator *= (Quat &lhs, Quat rhs);
Quat &operator *= (Quat &lhs, float rhs);
Quat &operator /= (Quat &lhs, float rhs);

#endif // __cplusplus

#endif // LINALG_H
