#ifndef LINALG_H
#define LINALG_H

//
// DECLARATION
//

#ifdef LINALG_IMPLEMENTATION
    #define LINALG_DEF
#else
    #define LINALG_DEF extern
#endif

typedef struct { float x, y; }       Vec2;
typedef struct { float x, y, z; }    Vec3;
typedef struct { float x, y, z, w; } Vec4;
typedef struct { float m[16]; }      Mat4;
typedef struct { float x, y, z, w; } Quat;

typedef struct {
    Vec3 position;
    Quat rotation;
    Vec3 scale;
} Transform;

LINALG_DEF Vec2 vec2_make(float x, float y);
LINALG_DEF Vec2 vec2_add(Vec2 a, Vec2 b);
LINALG_DEF Vec2 vec2_sub(Vec2 a, Vec2 b);
LINALG_DEF Vec2 vec2_mul(Vec2 a, Vec2 b);
LINALG_DEF Vec2 vec2_div(Vec2 a, Vec2 b);
LINALG_DEF Vec2 vec2_scale(Vec2 v, float s);
LINALG_DEF float vec2_dot(Vec2 a, Vec2 b);
LINALG_DEF float vec2_length(Vec2 v);
LINALG_DEF float vec2_length_sq(Vec2 v);
LINALG_DEF Vec2 vec2_norm(Vec2 v);

LINALG_DEF Vec2 operator + (Vec2 a, Vec2 b);
LINALG_DEF Vec2 operator - (Vec2 a, Vec2 b);
LINALG_DEF Vec2 operator * (Vec2 a, Vec2 b);
LINALG_DEF Vec2 operator / (Vec2 a, Vec2 b);
LINALG_DEF Vec2 operator * (Vec2 v, float s);

LINALG_DEF Vec3 vec3_make(float x, float y, float z);
LINALG_DEF Vec3 vec3_add(Vec3 a, Vec3 b);
LINALG_DEF Vec3 vec3_sub(Vec3 a, Vec3 b);
LINALG_DEF Vec3 vec3_mul(Vec3 a, Vec3 b);
LINALG_DEF Vec3 vec3_div(Vec3 a, Vec3 b);
LINALG_DEF Vec3 vec3_scale(Vec3 v, float s);
LINALG_DEF float vec3_dot(Vec3 a, Vec3 b);
LINALG_DEF Vec3 vec3_cross(Vec3 a, Vec3 b);
LINALG_DEF float vec3_length(Vec3 v);
LINALG_DEF float vec3_length_sq(Vec3 v);
LINALG_DEF Vec3 vec3_norm(Vec3 v);
LINALG_DEF Vec3 vec3_rotate_quat(Vec3 v, Quat q);

LINALG_DEF Vec3 operator + (Vec3 a, Vec3 b);
LINALG_DEF Vec3 operator - (Vec3 a, Vec3 b);
LINALG_DEF Vec3 operator * (Vec3 a, Vec3 b);
LINALG_DEF Vec3 operator / (Vec3 a, Vec3 b);
LINALG_DEF Vec3 operator * (Vec3 v, float s);

LINALG_DEF Vec4 vec4_make(float x, float y, float z, float w);
LINALG_DEF Vec4 vec4_add(Vec4 a, Vec4 b);
LINALG_DEF Vec4 vec4_sub(Vec4 a, Vec4 b);
LINALG_DEF Vec4 vec4_mul(Vec4 a, Vec4 b);
LINALG_DEF Vec4 vec4_div(Vec4 a, Vec4 b);
LINALG_DEF Vec4 vec4_scale(Vec4 v, float s);
LINALG_DEF float vec4_dot(Vec4 a, Vec4 b);
LINALG_DEF float vec4_length(Vec4 v);
LINALG_DEF float vec4_length_sq(Vec4 v);
LINALG_DEF Vec4 vec4_norm(Vec4 v);

LINALG_DEF Vec4 operator + (Vec4 a, Vec4 b);
LINALG_DEF Vec4 operator - (Vec4 a, Vec4 b);
LINALG_DEF Vec4 operator * (Vec4 a, Vec4 b);
LINALG_DEF Vec4 operator / (Vec4 a, Vec4 b);
LINALG_DEF Vec4 operator * (Vec4 v, float s);

// These are column-major, right-handed, column vectors,
// i.e. result = M * v, and transforms compose as M = T * R * S
LINALG_DEF Mat4 mat4_identity(void);
LINALG_DEF Mat4 mat4_mul(Mat4 a, Mat4 b);
LINALG_DEF Mat4 mat4_translate(Vec3 t);
LINALG_DEF Mat4 mat4_scale(Vec3 s);
LINALG_DEF Mat4 mat4_rotate_x(float rads);
LINALG_DEF Mat4 mat4_rotate_y(float rads);
LINALG_DEF Mat4 mat4_rotate_z(float rads);
LINALG_DEF Mat4 mat4_from_quat_make(Quat q);
LINALG_DEF Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);
LINALG_DEF Mat4 mat4_perspective(float fov_rads, float aspect, float near, float far);
LINALG_DEF Mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far);
LINALG_DEF Mat4 mat4_transpose(Mat4 m);
LINALG_DEF Mat4 mat4_inverse_affine(Mat4 m);

LINALG_DEF Mat4 operator * (Mat4 a, Mat4 b);

// Quaternion multiplication order matters. This implementation uses Hamilton product,
// where quat_mul(a, b) means apply b then a (same convention as mat4_mul).
LINALG_DEF Quat quat_make(float x, float y, float z, float w);
LINALG_DEF Quat quat_identity(void);
LINALG_DEF Quat quat_mul(Quat a, Quat b);
LINALG_DEF Quat quat_norm(Quat q);
LINALG_DEF Quat quat_from_axis_angle(Vec3 axis, float rads);
LINALG_DEF Quat quat_from_euler(float pitch, float yaw, float roll);
LINALG_DEF Mat4 quat_to_mat4(Quat q);
LINALG_DEF Quat quat_slerp(Quat a, Quat b, float t);

// transform_mul(a, b) should mean apply b first, then a, matching mat4_mul and quat_mul.
LINALG_DEF Transform transform_make(Vec3 pos, Quat rot, Vec3 scale);
LINALG_DEF Mat4 transform_to_mat4(Transform t);
LINALG_DEF Transform transform_mul(Transform a, Transform b);

//
// IMPLEMENTATION
//

#ifdef LINALG_IMPLEMENTATION

#include <math.h>

LINALG_DEF Vec2 vec2_make(float x, float y) {
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

LINALG_DEF Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2_make(a.x + b.x, a.y + b.y);
}

LINALG_DEF Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return vec2_make(a.x - b.x, a.y - b.y);
}

LINALG_DEF Vec2 vec2_mul(Vec2 a, Vec2 b) {
    return vec2_make(a.x * b.x, a.y * b.y);
}

LINALG_DEF Vec2 vec2_div(Vec2 a, Vec2 b) {
    return vec2_make(a.x / b.x, a.y / b.y);
}

LINALG_DEF Vec2 vec2_scale(Vec2 v, float s) {
    return vec2_make(v.x * s, v.y * s);
}

LINALG_DEF float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

LINALG_DEF float vec2_length_sq(Vec2 v) {
    return vec2_dot(v, v);
}

LINALG_DEF float vec2_length(Vec2 v) {
    return sqrtf(vec2_length_sq(v));
}

LINALG_DEF Vec2 vec2_norm(Vec2 v) {
    float len = vec2_length(v);
    if (len == 0.0f) return vec2_make(0.0f, 0.0f);
    return vec2_scale(v, 1.0f / len);
}

LINALG_DEF Vec2 operator + (Vec2 a, Vec2 b) {
    return vec2_add(a, b);
}

LINALG_DEF Vec2 operator - (Vec2 a, Vec2 b) {
    return vec2_sub(a, b);
}

LINALG_DEF Vec2 operator * (Vec2 a, Vec2 b) {
    return vec2_mul(a, b);
}

LINALG_DEF Vec2 operator / (Vec2 a, Vec2 b) {
    return vec2_div(a, b);
}

LINALG_DEF Vec2 operator * (Vec2 v, float s) {
    return vec2_scale(v, s);
}

LINALG_DEF Vec3 vec3_make(float x, float y, float z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

LINALG_DEF Vec3 vec3_add(Vec3 a, Vec3 b) {
    return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

LINALG_DEF Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

LINALG_DEF Vec3 vec3_mul(Vec3 a, Vec3 b) {
    return vec3_make(a.x * b.x, a.y * b.y, a.z * b.z);
}

LINALG_DEF Vec3 vec3_div(Vec3 a, Vec3 b) {
    return vec3_make(a.x / b.x, a.y / b.y, a.z / b.z);
}

LINALG_DEF Vec3 vec3_scale(Vec3 v, float s) {
    return vec3_make(v.x * s, v.y * s, v.z * s);
}

LINALG_DEF float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

LINALG_DEF Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return vec3_make(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

LINALG_DEF float vec3_length_sq(Vec3 v) {
    return vec3_dot(v, v);
}

LINALG_DEF float vec3_length(Vec3 v) {
    return sqrtf(vec3_length_sq(v));
}

LINALG_DEF Vec3 vec3_norm(Vec3 v) {
    float len = vec3_length(v);
    if (len == 0.0f) return vec3_make(0.0f, 0.0f, 0.0f);
    return vec3_scale(v, 1.0f / len);
}

LINALG_DEF Vec3 vec3_rotate_quat(Vec3 v, Quat q) {
    Quat vq = quat_make(v.x, v.y, v.z, 0.0f);
    Quat qi = quat_make(-q.x, -q.y, -q.z, q.w); // assuming normalized q
    Quat rq = quat_mul(quat_mul(q, vq), qi);
    return vec3_make(rq.x, rq.y, rq.z);
}

LINALG_DEF Vec3 operator + (Vec3 a, Vec3 b) {
    return vec3_add(a, b);
}

LINALG_DEF Vec3 operator - (Vec3 a, Vec3 b) {
    return vec3_sub(a, b);
}

LINALG_DEF Vec3 operator * (Vec3 a, Vec3 b) {
    return vec3_mul(a, b);
}

LINALG_DEF Vec3 operator / (Vec3 a, Vec3 b) {
    return vec3_div(a, b);
}

LINALG_DEF Vec3 operator * (Vec3 v, float s) {
    return vec3_scale(v, s);
}

LINALG_DEF Vec4 vec4_make(float x, float y, float z, float w) {
    Vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

LINALG_DEF Vec4 vec4_add(Vec4 a, Vec4 b) {
    return vec4_make(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

LINALG_DEF Vec4 vec4_sub(Vec4 a, Vec4 b) {
    return vec4_make(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

LINALG_DEF Vec4 vec4_mul(Vec4 a, Vec4 b) {
    return vec4_make(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

LINALG_DEF Vec4 vec4_div(Vec4 a, Vec4 b) {
    return vec4_make(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

LINALG_DEF Vec4 vec4_scale(Vec4 v, float s) {
    return vec4_make(v.x * s, v.y * s, v.z * s, v.w * s);
}

LINALG_DEF float vec4_dot(Vec4 a, Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

LINALG_DEF float vec4_length_sq(Vec4 v) {
    return vec4_dot(v, v);
}

LINALG_DEF float vec4_length(Vec4 v) {
    return sqrtf(vec4_length_sq(v));
}

LINALG_DEF Vec4 vec4_norm(Vec4 v) {
    float len = vec4_length(v);
    if (len == 0.0f) return vec4_make(0.0f, 0.0f, 0.0f, 0.0f);
    return vec4_scale(v, 1.0f / len);
}

LINALG_DEF Vec4 operator + (Vec4 a, Vec4 b) {
    return vec4_add(a, b);
}

LINALG_DEF Vec4 operator - (Vec4 a, Vec4 b) {
    return vec4_sub(a, b);
}

LINALG_DEF Vec4 operator * (Vec4 a, Vec4 b) {
    return vec4_mul(a, b);
}

LINALG_DEF Vec4 operator / (Vec4 a, Vec4 b) {
    return vec4_div(a, b);
}

LINALG_DEF Vec4 operator * (Vec4 v, float s) {
    return vec4_scale(v, s);
}

LINALG_DEF Mat4 mat4_identity(void) {
    Mat4 m = {0};
    m.m[0]  = 1.0f;
    m.m[5]  = 1.0f;
    m.m[10] = 1.0f;
    m.m[15] = 1.0f;
    return m;
}

LINALG_DEF Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 r;
    for (int c = 0; c < 4; ++c) {
        for (int r_i = 0; r_i < 4; ++r_i) {
            r.m[c*4 + r_i] =
                a.m[0*4 + r_i] * b.m[c*4 + 0] +
                a.m[1*4 + r_i] * b.m[c*4 + 1] +
                a.m[2*4 + r_i] * b.m[c*4 + 2] +
                a.m[3*4 + r_i] * b.m[c*4 + 3];
        }
    }
    return r;
}

LINALG_DEF Mat4 mat4_translate(Vec3 t) {
    Mat4 m = mat4_identity();
    m.m[12] = t.x;
    m.m[13] = t.y;
    m.m[14] = t.z;
    return m;
}

LINALG_DEF Mat4 mat4_scale(Vec3 s) {
    Mat4 m = {0};
    m.m[0]  = s.x;
    m.m[5]  = s.y;
    m.m[10] = s.z;
    m.m[15] = 1.0f;
    return m;
}

LINALG_DEF Mat4 mat4_rotate_x(float r) {
    float c = cosf(r);
    float s = sinf(r);

    Mat4 m = mat4_identity();
    m.m[5]  = c;
    m.m[6]  = s;
    m.m[9]  = -s;
    m.m[10] = c;
    return m;
}

LINALG_DEF Mat4 mat4_rotate_y(float r) {
    float c = cosf(r);
    float s = sinf(r);

    Mat4 m = mat4_identity();
    m.m[0]  = c;
    m.m[2]  = -s;
    m.m[8]  = s;
    m.m[10] = c;
    return m;
}

LINALG_DEF Mat4 mat4_rotate_z(float r) {
    float c = cosf(r);
    float s = sinf(r);

    Mat4 m = mat4_identity();
    m.m[0] = c;
    m.m[1] = s;
    m.m[4] = -s;
    m.m[5] = c;
    return m;
}

LINALG_DEF Mat4 mat4_from_quat_make(Quat q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    Mat4 m = mat4_identity();

    m.m[0]  = 1.0f - 2.0f * (yy + zz);
    m.m[1]  = 2.0f * (xy + wz);
    m.m[2]  = 2.0f * (xz - wy);

    m.m[4]  = 2.0f * (xy - wz);
    m.m[5]  = 1.0f - 2.0f * (xx + zz);
    m.m[6]  = 2.0f * (yz + wx);

    m.m[8]  = 2.0f * (xz + wy);
    m.m[9]  = 2.0f * (yz - wx);
    m.m[10] = 1.0f - 2.0f * (xx + yy);

    return m;
}

LINALG_DEF Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_norm(vec3_sub(center, eye));
    Vec3 s = vec3_norm(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 m = mat4_identity();

    m.m[0] = s.x;
    m.m[1] = u.x;
    m.m[2] = -f.x;

    m.m[4] = s.y;
    m.m[5] = u.y;
    m.m[6] = -f.y;

    m.m[8]  = s.z;
    m.m[9]  = u.z;
    m.m[10] = -f.z;

    m.m[12] = -vec3_dot(s, eye);
    m.m[13] = -vec3_dot(u, eye);
    m.m[14] =  vec3_dot(f, eye);

    return m;
}

LINALG_DEF Mat4 mat4_perspective(float fov, float aspect, float znear, float zfar) {
    float f = 1.0f / tanf(fov * 0.5f);

    Mat4 m = {0};
    m.m[0]  = f / aspect;
    m.m[5]  = f;
    m.m[10] = (zfar + znear) / (znear - zfar);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * zfar * znear) / (znear - zfar);
    return m;
}

LINALG_DEF Mat4 mat4_ortho(float l, float r, float b, float t, float n, float f) {
    Mat4 m = mat4_identity();

    m.m[0]  = 2.0f / (r - l);
    m.m[5]  = 2.0f / (t - b);
    m.m[10] = -2.0f / (f - n);

    m.m[12] = -(r + l) / (r - l);
    m.m[13] = -(t + b) / (t - b);
    m.m[14] = -(f + n) / (f - n);

    return m;
}

LINALG_DEF Mat4 mat4_transpose(Mat4 m) {
    Mat4 r;
    for (int c = 0; c < 4; ++c)
        for (int r_i = 0; r_i < 4; ++r_i)
            r.m[c*4 + r_i] = m.m[r_i*4 + c];
    return r;
}

LINALG_DEF Mat4 mat4_inverse_affine(Mat4 m) {
    Mat4 r;

    float a00 = m.m[0],  a01 = m.m[4],  a02 = m.m[8];
    float a10 = m.m[1],  a11 = m.m[5],  a12 = m.m[9];
    float a20 = m.m[2],  a21 = m.m[6],  a22 = m.m[10];

    float det =
        a00*(a11*a22 - a12*a21) -
        a01*(a10*a22 - a12*a20) +
        a02*(a10*a21 - a11*a20);

    if (fabsf(det) < 1e-8f) {
        // non-invertible → return identity as fallback
        return mat4_identity();
    }

    float inv_det = 1.0f / det;

    r.m[0]  =  (a11*a22 - a12*a21) * inv_det;
    r.m[4]  = -(a01*a22 - a02*a21) * inv_det;
    r.m[8]  =  (a01*a12 - a02*a11) * inv_det;

    r.m[1]  = -(a10*a22 - a12*a20) * inv_det;
    r.m[5]  =  (a00*a22 - a02*a20) * inv_det;
    r.m[9]  = -(a00*a12 - a02*a10) * inv_det;

    r.m[2]  =  (a10*a21 - a11*a20) * inv_det;
    r.m[6]  = -(a00*a21 - a01*a20) * inv_det;
    r.m[10] =  (a00*a11 - a01*a10) * inv_det;

    // last row
    r.m[3] = r.m[7] = r.m[11] = 0.0f;
    r.m[15] = 1.0f;

    // translation
    Vec3 t = vec3_make(m.m[12], m.m[13], m.m[14]);
    Vec3 ti = vec3_make(
        -(r.m[0]*t.x + r.m[4]*t.y + r.m[8]*t.z),
        -(r.m[1]*t.x + r.m[5]*t.y + r.m[9]*t.z),
        -(r.m[2]*t.x + r.m[6]*t.y + r.m[10]*t.z)
    );

    r.m[12] = ti.x;
    r.m[13] = ti.y;
    r.m[14] = ti.z;

    return r;
}

LINALG_DEF Mat4 operator * (Mat4 a, Mat4 b) {
    return mat4_mul(a, b);
}

LINALG_DEF Quat quat_make(float x, float y, float z, float w) {
    Quat q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

LINALG_DEF Quat quat_identity(void) {
    return quat_make(0.0f, 0.0f, 0.0f, 1.0f);
}

LINALG_DEF Quat quat_mul(Quat a, Quat b) {
    return quat_make(
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z
    );
}

LINALG_DEF Quat quat_norm(Quat q) {
    float len = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (len == 0.0f) return quat_identity();
    float inv = 1.0f / len;
    return quat_make(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
}

LINALG_DEF Quat quat_from_axis_angle(Vec3 axis, float rads) {
    Vec3 n = vec3_norm(axis);
    float half = rads * 0.5f;
    float s = sinf(half);
    float c = cosf(half);

    return quat_make(n.x * s, n.y * s, n.z * s, c);
}

// Convention (important):
//   - pitch = rotation around X
//   - yaw   = rotation around Y
//   - roll  = rotation around Z
//   - order = roll * yaw * pitch (Z * Y * X)
LINALG_DEF Quat quat_from_euler(float pitch, float yaw, float roll) {
    float hx = pitch * 0.5f;
    float hy = yaw   * 0.5f;
    float hz = roll  * 0.5f;

    float sx = sinf(hx), cx = cosf(hx);
    float sy = sinf(hy), cy = cosf(hy);
    float sz = sinf(hz), cz = cosf(hz);

    Quat qx = quat_make(sx, 0.0f, 0.0f, cx);
    Quat qy = quat_make(0.0f, sy, 0.0f, cy);
    Quat qz = quat_make(0.0f, 0.0f, sz, cz);

    return quat_mul(qz, quat_mul(qy, qx));
}

LINALG_DEF Mat4 quat_to_mat4(Quat q) {
    // same as mat4_from_quat, but kept for API symmetry
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    Mat4 m = mat4_identity();

    m.m[0]  = 1.0f - 2.0f * (yy + zz);
    m.m[1]  = 2.0f * (xy + wz);
    m.m[2]  = 2.0f * (xz - wy);

    m.m[4]  = 2.0f * (xy - wz);
    m.m[5]  = 1.0f - 2.0f * (xx + zz);
    m.m[6]  = 2.0f * (yz + wx);

    m.m[8]  = 2.0f * (xz + wy);
    m.m[9]  = 2.0f * (yz - wx);
    m.m[10] = 1.0f - 2.0f * (xx + yy);

    return m;
}

LINALG_DEF Quat quat_slerp(Quat a, Quat b, float t) {
    // Compute cosine of angle between them
    float dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;

    if (dot < 0.0f) {
        // Use shortest path
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
        b.w = -b.w;
        dot = -dot;
    }

    const float DOT_THRESHOLD = 0.9995f;

    if (dot > DOT_THRESHOLD) {
        // Use linear interpolation
        Quat result = quat_make(
            a.x + t*(b.x - a.x),
            a.y + t*(b.y - a.y),
            a.z + t*(b.z - a.z),
            a.w + t*(b.w - a.w)
        );
        return quat_norm(result);
    }

    float theta_0 = acosf(dot); // angle between a and b
    float theta   = theta_0 * t;

    float sin_theta   = sinf(theta);
    float sin_theta_0 = sinf(theta_0);

    float s0 = cosf(theta) - dot * sin_theta / sin_theta_0;
    float s1 = sin_theta / sin_theta_0;

    return quat_make(
        (a.x * s0) + (b.x * s1),
        (a.y * s0) + (b.y * s1),
        (a.z * s0) + (b.z * s1),
        (a.w * s0) + (b.w * s1)
    );
}

LINALG_DEF Transform transform_make(Vec3 pos, Quat rot, Vec3 scale) {
    Transform t;
    t.position = pos;
    t.rotation = rot;
    t.scale    = scale;
    return t;
}

LINALG_DEF Mat4 transform_to_mat4(Transform t) {
    Mat4 T = mat4_translate(t.position);
    Mat4 R = mat4_from_quat_make(t.rotation);
    Mat4 S = mat4_scale(t.scale);

    // column-major, column vectors: M = T * R * S
    return mat4_mul(T, mat4_mul(R, S));
}

LINALG_DEF Transform transform_mul(Transform a, Transform b) {
    Transform r;

    r.scale = vec3_mul(a.scale, b.scale);
    r.rotation = quat_mul(a.rotation, b.rotation);

    // position: scale -> rotate -> translate
    Vec3 p = vec3_mul(a.scale, b.position);
    p = vec3_rotate_quat(p, a.rotation);
    r.position = vec3_add(a.position, p);

    return r;
}

#endif // LINALG_IMPLEMENTATION

#endif // LINALG_H
