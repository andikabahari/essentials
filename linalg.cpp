#include "linalg.h"

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool __feq(float a, float b) {
    return fabsf(a - b) <= LINALG_EPSILON;
}

Vec2 vec2_make(float x, float y) {
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

bool vec2_eq(Vec2 a, Vec2 b) {
    return __feq(a.x, b.x) && __feq(a.y, b.y);
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2_make(a.x + b.x, a.y + b.y);
}

Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return vec2_make(a.x - b.x, a.y - b.y);
}

Vec2 vec2_neg(Vec2 v) {
    return vec2_make(-v.x, -v.y);
}

Vec2 vec2_mul(Vec2 a, Vec2 b) {
    return vec2_make(a.x * b.x, a.y * b.y);
}

Vec2 vec2_div(Vec2 a, Vec2 b) {
    return vec2_make(a.x / b.x, a.y / b.y);
}

Vec2 vec2_scale(Vec2 v, float s) {
    return vec2_make(v.x * s, v.y * s);
}

float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float vec2_length_sq(Vec2 v) {
    return vec2_dot(v, v);
}

float vec2_length(Vec2 v) {
    return sqrtf(vec2_length_sq(v));
}

Vec2 vec2_norm(Vec2 v) {
    float len = vec2_length(v);
    if (len == 0.0f) return vec2_make(0.0f, 0.0f);
    return vec2_scale(v, 1.0f / len);
}

Vec3 vec3_make(float x, float y, float z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

bool vec3_eq(Vec3 a, Vec3 b) {
    return __feq(a.x, b.x) && __feq(a.y, b.y) && __feq(a.z, b.z);
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 vec3_neg(Vec3 v) {
    return vec3_make(-v.x, -v.y, -v.z);
}

Vec3 vec3_mul(Vec3 a, Vec3 b) {
    return vec3_make(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec3 vec3_div(Vec3 a, Vec3 b) {
    return vec3_make(a.x / b.x, a.y / b.y, a.z / b.z);
}

Vec3 vec3_scale(Vec3 v, float s) {
    return vec3_make(v.x * s, v.y * s, v.z * s);
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return vec3_make(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float vec3_length_sq(Vec3 v) {
    return vec3_dot(v, v);
}

float vec3_length(Vec3 v) {
    return sqrtf(vec3_length_sq(v));
}

Vec3 vec3_norm(Vec3 v) {
    float len = vec3_length(v);
    if (len == 0.0f) return vec3_make(0.0f, 0.0f, 0.0f);
    return vec3_scale(v, 1.0f / len);
}

Vec3 vec3_rotate_quat(Vec3 v, Quat q) {
    Quat vq = quat_make(v.x, v.y, v.z, 0.0f);
    Quat qi = quat_make(-q.x, -q.y, -q.z, q.w); // assuming normalized q
    Quat rq = quat_mul(quat_mul(q, vq), qi);
    return vec3_make(rq.x, rq.y, rq.z);
}

Vec4 vec4_make(float x, float y, float z, float w) {
    Vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

bool vec4_eq(Vec4 a, Vec4 b) {
    return __feq(a.x, b.x) &&
           __feq(a.y, b.y) &&
           __feq(a.z, b.z) &&
           __feq(a.w, b.w);
}

Vec4 vec4_add(Vec4 a, Vec4 b) {
    return vec4_make(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Vec4 vec4_sub(Vec4 a, Vec4 b) {
    return vec4_make(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Vec4 vec4_neg(Vec4 v) {
    return vec4_make(-v.x, -v.y, -v.z, -v.w);
}

Vec4 vec4_mul(Vec4 a, Vec4 b) {
    return vec4_make(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

Vec4 vec4_div(Vec4 a, Vec4 b) {
    return vec4_make(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

Vec4 vec4_scale(Vec4 v, float s) {
    return vec4_make(v.x * s, v.y * s, v.z * s, v.w * s);
}

float vec4_dot(Vec4 a, Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float vec4_length_sq(Vec4 v) {
    return vec4_dot(v, v);
}

float vec4_length(Vec4 v) {
    return sqrtf(vec4_length_sq(v));
}

Vec4 vec4_norm(Vec4 v) {
    float len = vec4_length(v);
    if (len == 0.0f) return vec4_make(0.0f, 0.0f, 0.0f, 0.0f);
    return vec4_scale(v, 1.0f / len);
}

Mat4 mat4_identity(void) {
    Mat4 m = {0};
    m.m[MAT4_00] = 1.0f;
    m.m[MAT4_11] = 1.0f;
    m.m[MAT4_22] = 1.0f;
    m.m[MAT4_33] = 1.0f;
    return m;
}

bool mat4_eq(Mat4 a, Mat4 b) {
    for (int i = 0; i < 16; ++i) {
        if (!__feq(a.m[i], b.m[i])) return false;
    }
    return true;
}

Mat4 mat4_add(Mat4 a, Mat4 b) {
    Mat4 m;

    for (int i = 0; i < 16; ++i) {
        m.m[i] = a.m[i] + b.m[i];
    }

    return m;
}

Mat4 mat4_sub(Mat4 a, Mat4 b) {
    Mat4 m;

    for (int i = 0; i < 16; ++i) {
        m.m[i] = a.m[i] - b.m[i];
    }

    return m;
}

Mat4 mat4_neg(Mat4 m) {
    Mat4 r;

    for (int i = 0; i < 16; ++i) {
        r.m[i] = -m.m[i];
    }

    return r;
}

Mat4 mat4_mul(Mat4 a, Mat4 b) {
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

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
    Vec4 r;

    r.x =
        m.m[MAT4_00] * v.x +
        m.m[MAT4_01] * v.y +
        m.m[MAT4_02] * v.z +
        m.m[MAT4_03] * v.w;

    r.y =
        m.m[MAT4_10] * v.x +
        m.m[MAT4_11] * v.y +
        m.m[MAT4_12] * v.z +
        m.m[MAT4_13] * v.w;

    r.z =
        m.m[MAT4_20] * v.x +
        m.m[MAT4_21] * v.y +
        m.m[MAT4_22] * v.z +
        m.m[MAT4_23] * v.w;

    r.w =
        m.m[MAT4_30] * v.x +
        m.m[MAT4_31] * v.y +
        m.m[MAT4_32] * v.z +
        m.m[MAT4_33] * v.w;

    return r;
}

Mat4 mat4_mul_scalar(Mat4 m, float s) {
    Mat4 r;
    for (int i = 0; i < 16; i++) {
        r.m[i] = m.m[i] * s;
    }
    return r;
}

Mat4 mat4_translate(Vec3 t) {
    Mat4 m = mat4_identity();
    m.m[MAT4_03] = t.x;
    m.m[MAT4_13] = t.y;
    m.m[MAT4_23] = t.z;
    return m;
}

Mat4 mat4_scaling(Vec3 s) {
    Mat4 m = {0};
    m.m[MAT4_00] = s.x;
    m.m[MAT4_11] = s.y;
    m.m[MAT4_22] = s.z;
    m.m[MAT4_33] = 1.0f;
    return m;
}

Mat4 mat4_rotate(float rads, Vec3 axis) {
    float c = cosf(rads);
    float s = sinf(rads);

    Vec3 a = vec3_norm(axis);
    Vec3 t = vec3_scale(a, 1.0f - c);

    Mat4 rot = mat4_identity();

    // Column 0
    rot.m[MAT4_00] = c + t.x * a.x;
    rot.m[MAT4_10] = t.x * a.y + s * a.z;
    rot.m[MAT4_20] = t.x * a.z - s * a.y;
    rot.m[MAT4_30] = 0.0f;

    // Column 1
    rot.m[MAT4_01] = t.y * a.x - s * a.z;
    rot.m[MAT4_11] = c + t.y * a.y;
    rot.m[MAT4_21] = t.y * a.z + s * a.x;
    rot.m[MAT4_31] = 0.0f;

    // Column 2
    rot.m[MAT4_02] = t.z * a.x + s * a.y;
    rot.m[MAT4_12] = t.z * a.y - s * a.x;
    rot.m[MAT4_22] = c + t.z * a.z;
    rot.m[MAT4_32] = 0.0f;

    return rot;
}

Mat4 mat4_from_quat(Quat q) {
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

    m.m[MAT4_00] = 1.0f - 2.0f * (yy + zz);
    m.m[MAT4_10] = 2.0f * (xy + wz);
    m.m[MAT4_20] = 2.0f * (xz - wy);

    m.m[MAT4_01] = 2.0f * (xy - wz);
    m.m[MAT4_11] = 1.0f - 2.0f * (xx + zz);
    m.m[MAT4_21] = 2.0f * (yz + wx);

    m.m[MAT4_02] = 2.0f * (xz + wy);
    m.m[MAT4_12] = 2.0f * (yz - wx);
    m.m[MAT4_22] = 1.0f - 2.0f * (xx + yy);

    return m;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_norm(vec3_sub(center, eye));
    Vec3 s = vec3_norm(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 m = mat4_identity();

    m.m[MAT4_00] = s.x;
    m.m[MAT4_10] = u.x;
    m.m[MAT4_20] = -f.x;

    m.m[MAT4_01] = s.y;
    m.m[MAT4_11] = u.y;
    m.m[MAT4_21] = -f.y;

    m.m[MAT4_02] = s.z;
    m.m[MAT4_12] = u.z;
    m.m[MAT4_22] = -f.z;

    m.m[MAT4_03] = -vec3_dot(s, eye);
    m.m[MAT4_13] = -vec3_dot(u, eye);
    m.m[MAT4_23] =  vec3_dot(f, eye);

    return m;
}

Mat4 mat4_perspective(float fov, float aspect, float znear, float zfar) {
    float f = 1.0f / tanf(fov * 0.5f);

    Mat4 m = {0};
    m.m[MAT4_00] = f / aspect;
    m.m[MAT4_11] = f;
    m.m[MAT4_22] = (zfar + znear) / (znear - zfar);
    m.m[MAT4_32] = -1.0f;
    m.m[MAT4_23] = (2.0f * zfar * znear) / (znear - zfar);
    return m;
}

Mat4 mat4_ortho(float l, float r, float b, float t, float n, float f) {
    Mat4 m = mat4_identity();

    m.m[MAT4_00] = 2.0f / (r - l);
    m.m[MAT4_11] = 2.0f / (t - b);
    m.m[MAT4_22] = -2.0f / (f - n);

    m.m[MAT4_03] = -(r + l) / (r - l);
    m.m[MAT4_13] = -(t + b) / (t - b);
    m.m[MAT4_23] = -(f + n) / (f - n);

    return m;
}

Mat4 mat4_transpose(Mat4 m) {
    Mat4 r;
    for (int c = 0; c < 4; ++c)
        for (int r_i = 0; r_i < 4; ++r_i)
            r.m[c*4 + r_i] = m.m[r_i*4 + c];
    return r;
}

Mat4 mat4_inverse(Mat4 m) {
    Mat4 r;

    float a00 = m.m[MAT4_00], a01 = m.m[MAT4_01], a02 = m.m[MAT4_02];
    float a10 = m.m[MAT4_10], a11 = m.m[MAT4_11], a12 = m.m[MAT4_12];
    float a20 = m.m[MAT4_20], a21 = m.m[MAT4_21], a22 = m.m[MAT4_22];

    float det =
        a00*(a11*a22 - a12*a21) -
        a01*(a10*a22 - a12*a20) +
        a02*(a10*a21 - a11*a20);

    if (fabsf(det) < 1e-8f) {
        return mat4_identity();
    }

    float inv_det = 1.0f / det;

    r.m[MAT4_00] =  (a11*a22 - a12*a21) * inv_det;
    r.m[MAT4_01] = -(a01*a22 - a02*a21) * inv_det;
    r.m[MAT4_02] =  (a01*a12 - a02*a11) * inv_det;

    r.m[MAT4_10] = -(a10*a22 - a12*a20) * inv_det;
    r.m[MAT4_11] =  (a00*a22 - a02*a20) * inv_det;
    r.m[MAT4_12] = -(a00*a12 - a02*a10) * inv_det;

    r.m[MAT4_20] =  (a10*a21 - a11*a20) * inv_det;
    r.m[MAT4_21] = -(a00*a21 - a01*a20) * inv_det;
    r.m[MAT4_22] =  (a00*a11 - a01*a10) * inv_det;

    // last row
    r.m[MAT4_30] = r.m[MAT4_31] = r.m[MAT4_32] = 0.0f;
    r.m[MAT4_33] = 1.0f;

    // translation
    Vec3 t = vec3_make(m.m[MAT4_03], m.m[MAT4_13], m.m[MAT4_23]);
    Vec3 ti = vec3_make(
        -(r.m[MAT4_00]*t.x + r.m[MAT4_01]*t.y + r.m[MAT4_02]*t.z),
        -(r.m[MAT4_10]*t.x + r.m[MAT4_11]*t.y + r.m[MAT4_12]*t.z),
        -(r.m[MAT4_20]*t.x + r.m[MAT4_21]*t.y + r.m[MAT4_22]*t.z)
    );

    r.m[MAT4_03] = ti.x;
    r.m[MAT4_13] = ti.y;
    r.m[MAT4_23] = ti.z;

    return r;
}

Quat quat_make(float x, float y, float z, float w) {
    Quat q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

Quat quat_identity(void) {
    return quat_make(0.0f, 0.0f, 0.0f, 1.0f);
}

bool quat_eq(Quat a, Quat b) {
    return __feq(a.x, b.x) &&
           __feq(a.y, b.y) &&
           __feq(a.z, b.z) &&
           __feq(a.w, b.w);
}

Quat quat_add(Quat a, Quat b) {
    Quat r;

    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    r.w = a.w + b.w;

    return r;
}

Quat quat_sub(Quat a, Quat b) {
    Quat r;

    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    r.w = a.w - b.w;

    return r;
}

Quat quat_neg(Quat q) {
    Quat r;

    r.x = -q.x;
    r.y = -q.y;
    r.z = -q.z;
    r.w = -q.w;

    return r;
}

Quat quat_scale(Quat q, float s) {
    Quat r;

    r.x = q.x * s;
    r.y = q.y * s;
    r.z = q.z * s;
    r.w = q.w * s;

    return r;
}

Quat quat_mul(Quat a, Quat b) {
    return quat_make(
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z
    );
}

Vec3 quat_mul_vec3(Quat q, Vec3 v) {
    Vec3 qv = vec3_make(q.x, q.y, q.z);

    Vec3 t = vec3_scale(vec3_cross(qv, v), 2.0f);

    return vec3_add(
        vec3_add(v, vec3_scale(t, q.w)),
        vec3_cross(qv, t)
    );
}

Quat quat_norm(Quat q) {
    float len = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (len == 0.0f) return quat_identity();
    float inv = 1.0f / len;
    return quat_make(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
}

Quat quat_from_axis_angle(Vec3 axis, float rads) {
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
Quat quat_from_euler(float pitch, float yaw, float roll) {
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

Mat4 quat_to_mat4(Quat q) {
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

    m.m[MAT4_00] = 1.0f - 2.0f * (yy + zz);
    m.m[MAT4_10] = 2.0f * (xy + wz);
    m.m[MAT4_20] = 2.0f * (xz - wy);

    m.m[MAT4_01] = 2.0f * (xy - wz);
    m.m[MAT4_11] = 1.0f - 2.0f * (xx + zz);
    m.m[MAT4_21] = 2.0f * (yz + wx);

    m.m[MAT4_02] = 2.0f * (xz + wy);
    m.m[MAT4_12] = 2.0f * (yz - wx);
    m.m[MAT4_22] = 1.0f - 2.0f * (xx + yy);

    return m;
}

Quat quat_slerp(Quat a, Quat b, float t) {
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

Transform transform_make(Vec3 pos, Quat rot, Vec3 scale) {
    Transform t;
    t.position = pos;
    t.rotation = rot;
    t.scale    = scale;
    return t;
}

Mat4 transform_to_mat4(Transform t) {
    Mat4 T = mat4_translate(t.position);
    Mat4 R = mat4_from_quat(t.rotation);
    Mat4 S = mat4_scaling(t.scale);

    // column-major, column vectors: M = T * R * S
    return mat4_mul(T, mat4_mul(R, S));
}

Transform transform_mul(Transform a, Transform b) {
    Transform r;

    r.scale = vec3_mul(a.scale, b.scale);
    r.rotation = quat_mul(a.rotation, b.rotation);

    // position: scale -> rotate -> translate
    Vec3 p = vec3_mul(a.scale, b.position);
    p = vec3_rotate_quat(p, a.rotation);
    r.position = vec3_add(a.position, p);

    return r;
}

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

bool operator == (Vec2 a, Vec2 b) {
    return vec2_eq(a, b);
}

bool operator != (Vec2 a, Vec2 b) {
    return !vec2_eq(a, b);
}

Vec2 operator + (Vec2 a, Vec2 b) {
    return vec2_add(a, b);
}

Vec2 operator - (Vec2 a, Vec2 b) {
    return vec2_sub(a, b);
}

Vec2 operator - (Vec2 v) {
    return vec2_neg(v);
}

Vec2 operator * (Vec2 a, Vec2 b) {
    return vec2_mul(a, b);
}

Vec2 operator / (Vec2 a, Vec2 b) {
    return vec2_div(a, b);
}

Vec2 operator * (Vec2 v, float s) {
    return vec2_scale(v, s);
}

Vec2 operator * (float s, Vec2 v) {
    return vec2_scale(v, s);
}

Vec2 operator / (Vec2 v, float s) {
    return vec2_scale(v, 1.0f/s);
}

Vec2 &operator += (Vec2 &lhs, Vec2 rhs) {
    lhs = vec2_add(lhs, rhs);
    return lhs;
}

Vec2 &operator -= (Vec2 &lhs, Vec2 rhs) {
    lhs = vec2_sub(lhs, rhs);
    return lhs;
}

Vec2 &operator *= (Vec2 &lhs, Vec2 rhs) {
    lhs = vec2_mul(lhs, rhs);
    return lhs;
}

Vec2 &operator *= (Vec2 &lhs, float rhs) {
    lhs = vec2_scale(lhs, rhs);
    return lhs;
}

Vec2 &operator /= (Vec2 &lhs, Vec2 rhs) {
    lhs = vec2_div(lhs, rhs);
    return lhs;
}

Vec2 &operator /= (Vec2 &lhs, float rhs) {
    lhs = vec2_scale(lhs, 1.0f/rhs);
    return lhs;
}

bool operator == (Vec3 a, Vec3 b) {
    return vec3_eq(a, b);
}

bool operator != (Vec3 a, Vec3 b) {
    return !vec3_eq(a, b);
}

Vec3 operator + (Vec3 a, Vec3 b) {
    return vec3_add(a, b);
}

Vec3 operator - (Vec3 a, Vec3 b) {
    return vec3_sub(a, b);
}

Vec3 operator - (Vec3 v) {
    return vec3_neg(v);
}

Vec3 operator * (Vec3 a, Vec3 b) {
    return vec3_mul(a, b);
}

Vec3 operator / (Vec3 a, Vec3 b) {
    return vec3_div(a, b);
}

Vec3 operator * (Vec3 v, float s) {
    return vec3_scale(v, s);
}

Vec3 operator * (float s, Vec3 v) {
    return vec3_scale(v, s);
}

Vec3 operator / (Vec3 v, float s) {
    return vec3_scale(v, 1.0f/s);
}

Vec3 &operator += (Vec3 &lhs, Vec3 rhs) {
    lhs = vec3_add(lhs, rhs);
    return lhs;
}

Vec3 &operator -= (Vec3 &lhs, Vec3 rhs) {
    lhs = vec3_sub(lhs, rhs);
    return lhs;
}

Vec3 &operator *= (Vec3 &lhs, Vec3 rhs) {
    lhs = vec3_mul(lhs, rhs);
    return lhs;
}

Vec3 &operator *= (Vec3 &lhs, float rhs) {
    lhs = vec3_scale(lhs, rhs);
    return lhs;
}

Vec3 &operator /= (Vec3 &lhs, Vec3 rhs) {
    lhs = vec3_div(lhs, rhs);
    return lhs;
}

Vec3 &operator /= (Vec3 &lhs, float rhs) {
    lhs = vec3_scale(lhs, 1.0f/rhs);
    return lhs;
}

bool operator == (Vec4 a, Vec4 b) {
    return vec4_eq(a, b);
}

bool operator != (Vec4 a, Vec4 b) {
    return !vec4_eq(a, b);
}

Vec4 operator + (Vec4 a, Vec4 b) {
    return vec4_add(a, b);
}

Vec4 operator - (Vec4 a, Vec4 b) {
    return vec4_sub(a, b);
}

Vec4 operator - (Vec4 v) {
    return vec4_neg(v);
}

Vec4 operator * (Vec4 a, Vec4 b) {
    return vec4_mul(a, b);
}

Vec4 operator / (Vec4 a, Vec4 b) {
    return vec4_div(a, b);
}

Vec4 operator * (Vec4 v, float s) {
    return vec4_scale(v, s);
}

Vec4 operator * (float s, Vec4 v) {
    return vec4_scale(v, s);
}

Vec4 operator / (Vec4 v, float s) {
    return vec4_scale(v, 1.0f/s);
}

Vec4 &operator += (Vec4 &lhs, Vec4 rhs) {
    lhs = vec4_add(lhs, rhs);
    return lhs;
}

Vec4 &operator -= (Vec4 &lhs, Vec4 rhs) {
    lhs = vec4_sub(lhs, rhs);
    return lhs;
}

Vec4 &operator *= (Vec4 &lhs, Vec4 rhs) {
    lhs = vec4_mul(lhs, rhs);
    return lhs;
}

Vec4 &operator *= (Vec4 &lhs, float rhs) {
    lhs = vec4_scale(lhs, rhs);
    return lhs;
}

Vec4 &operator /= (Vec4 &lhs, Vec4 rhs) {
    lhs = vec4_div(lhs, rhs);
    return lhs;
}

Vec4 &operator /= (Vec4 &lhs, float rhs) {
    lhs = vec4_scale(lhs, 1.0f/rhs);
    return lhs;
}

bool operator == (Mat4 a, Mat4 b) {
    return mat4_eq(a, b);
}

bool operator != (Mat4 a, Mat4 b) {
    return !mat4_eq(a, b);
}

Mat4 operator + (Mat4 a, Mat4 b) {
    return mat4_add(a, b);
}

Mat4 operator - (Mat4 a, Mat4 b) {
    return mat4_sub(a, b);
}

Mat4 operator - (Mat4 m) {
    return mat4_neg(m);
}

Mat4 operator * (Mat4 a, Mat4 b) {
    return mat4_mul(a, b);
}

Vec4 operator * (Mat4 m, Vec4 v) {
    return mat4_mul_vec4(m, v);
}

Mat4 operator * (Mat4 m, float s) {
    return mat4_mul_scalar(m, s);
}

Mat4 operator * (float s, Mat4 m) {
    return mat4_mul_scalar(m, s);
}

Mat4 operator / (Mat4 m, float s) {
    return mat4_mul_scalar(m, 1.0f/s);
}

Mat4 &operator += (Mat4 &lhs, Mat4 rhs) {
    lhs = mat4_add(lhs, rhs);
    return lhs;
}

Mat4 &operator -= (Mat4 &lhs, Mat4 rhs) {
    lhs = mat4_sub(lhs, rhs);
    return lhs;
}

Mat4 &operator *= (Mat4 &lhs, Mat4 rhs) {
    lhs = mat4_mul(lhs, rhs);
    return lhs;
}

Mat4 &operator *= (Mat4 &lhs, float rhs) {
    lhs = mat4_mul_scalar(lhs, rhs);
    return lhs;
}

Mat4 &operator /= (Mat4 &lhs, float rhs) {
    lhs = mat4_mul_scalar(lhs, 1.0f/rhs);
    return lhs;
}

bool operator == (Quat a, Quat b) {
    return quat_eq(a, b);
}

bool operator != (Quat a, Quat b) {
    return !quat_eq(a, b);
}

Quat operator + (Quat a, Quat b) {
    return quat_add(a, b);
}

Quat operator - (Quat a, Quat b) {
    return quat_sub(a, b);
}

Quat operator - (Quat q) {
    return quat_neg(q);
}

Quat operator * (Quat a, Quat b) {
    return quat_mul(a, b);
}

Vec3 operator * (Quat q, Vec3 v) {
    return quat_mul_vec3(q, v);
}

Quat operator * (Quat q, float s) {
    return quat_scale(q, s);
}

Quat operator * (float s, Quat q) {
    return quat_scale(q, s);
}

Quat operator / (Quat q, float s) {
    return quat_scale(q, 1.0f/s);
}

Quat &operator += (Quat &lhs, Quat rhs) {
    lhs = quat_add(lhs, rhs);
    return lhs;
}

Quat &operator -= (Quat &lhs, Quat rhs) {
    lhs = quat_sub(lhs, rhs);
    return lhs;
}

Quat &operator *= (Quat &lhs, Quat rhs) {
    lhs = quat_mul(lhs, rhs);
    return lhs;
}

Quat &operator *= (Quat &lhs, float rhs) {
    lhs = quat_scale(lhs, rhs);
    return lhs;
}

Quat &operator /= (Quat &lhs, float rhs) {
    lhs = quat_scale(lhs, 1.0f/rhs);
    return lhs;
}

#endif // __cplusplus
