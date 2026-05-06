const float EPS = 1e-5f;
const float M_PI = 3.1415926f;

inline int feq(float a, float b) {
    return fabsf(a - b) < EPS;
}

inline int vec2_eq(Vec2 a, Vec2 b) {
    return feq(a.x, b.x) && feq(a.y, b.y);
}

inline int vec3_eq(Vec3 a, Vec3 b) {
    return feq(a.x, b.x) && feq(a.y, b.y) && feq(a.z, b.z);
}

inline int vec4_eq(Vec4 a, Vec4 b) {
    return feq(a.x, b.x) && feq(a.y, b.y) &&
           feq(a.z, b.z) && feq(a.w, b.w);
}

inline int mat4_eq(Mat4 a, Mat4 b) {
    for (int i = 0; i < 16; ++i) {
        if (!feq(a.m[i], b.m[i])) return 0;
    }
    return 1;
}

inline int mat4_is_identity(Mat4 m) {
    Mat4 I = mat4_identity();
    for (int i = 0; i < 16; ++i) {
        if (!feq(m.m[i], I.m[i])) return 0;
    }
    return 1;
}

inline Vec3 vec3_transform_point(Mat4 m, Vec3 v) {
    float x = v.x, y = v.y, z = v.z;

    float rx = m.m[0]*x + m.m[4]*y + m.m[8]*z  + m.m[12];
    float ry = m.m[1]*x + m.m[5]*y + m.m[9]*z  + m.m[13];
    float rz = m.m[2]*x + m.m[6]*y + m.m[10]*z + m.m[14];
    float rw = m.m[3]*x + m.m[7]*y + m.m[11]*z + m.m[15];

    if (rw != 0.0f) {
        rx /= rw; ry /= rw; rz /= rw;
    }

    return vec3_make(rx, ry, rz);
}

TEST(test_vec2) {
    Vec2 a = vec2_make(2, 4);
    Vec2 b = vec2_make(1, 2);

    ASSERT(vec2_eq(vec2_add(a, b), vec2_make(3, 6)));
    ASSERT(vec2_eq(vec2_sub(a, b), vec2_make(1, 2)));
    ASSERT(vec2_eq(vec2_mul(a, b), vec2_make(2, 8)));
    ASSERT(vec2_eq(vec2_div(a, b), vec2_make(2, 2)));

    ASSERT(feq(vec2_dot(a, b), 10.0f));
    ASSERT(feq(vec2_length_sq(a), 20.0f));
}

TEST(test_vec2_ops) {
    Vec2 a = vec2_make(2, 4);
    Vec2 b = vec2_make(1, 2);

    ASSERT(vec2_eq(a + b, vec2_make(3, 6)));
    ASSERT(vec2_eq(a - b, vec2_make(1, 2)));
    ASSERT(vec2_eq(a * b, vec2_make(2, 8)));
    ASSERT(vec2_eq(a / b, vec2_make(2, 2)));
    ASSERT(vec2_eq(a * 2, vec2_make(4, 8)));
}

TEST(test_vec3) {
    Vec3 a = vec3_make(1, 0, 0);
    Vec3 b = vec3_make(0, 1, 0);

    ASSERT(vec3_eq(vec3_cross(a, b), vec3_make(0, 0, 1)));
    ASSERT(feq(vec3_dot(a, b), 0.0f));

    Vec3 c = vec3_make(2, 0, 0);
    ASSERT(feq(vec3_length(c), 2.0f));
    ASSERT(vec3_eq(vec3_norm(c), vec3_make(1, 0, 0)));
}

TEST(test_vec3_ops) {
    Vec3 a = vec3_make(2, 4, 6);
    Vec3 b = vec3_make(1, 2, 3);

    ASSERT(vec3_eq(a + b, vec3_make(3, 6,  9)));
    ASSERT(vec3_eq(a - b, vec3_make(1, 2,  3)));
    ASSERT(vec3_eq(a * b, vec3_make(2, 8, 18)));
    ASSERT(vec3_eq(a / b, vec3_make(2, 2,  2)));
    ASSERT(vec3_eq(a * 2, vec3_make(4, 8, 12)));
}

TEST(test_vec3_cross_orthogonality) {
    Vec3 a = vec3_make(1,2,3);
    Vec3 b = vec3_make(4,5,6);

    Vec3 c = vec3_cross(a, b);

    ASSERT(feq(vec3_dot(c, a), 0.0f));
    ASSERT(feq(vec3_dot(c, b), 0.0f));
}

TEST(test_vec3_norm_idempotence) {
    Vec3 v = vec3_make(5,0,0);

    Vec3 n1 = vec3_norm(v);
    Vec3 n2 = vec3_norm(n1);

    ASSERT(vec3_eq(n1, n2));
}

TEST(test_vec4) {
    Vec4 a = vec4_make(2, 4, 6, 8);
    Vec4 b = vec4_make(1, 2, 3, 4);

    ASSERT(vec4_eq(vec4_mul(a, b), vec4_make(2, 8, 18, 32)));
    ASSERT(vec4_eq(vec4_div(a, b), vec4_make(2, 2, 2, 2)));

    ASSERT(feq(vec4_dot(a, b), 60.0f));
}

TEST(test_vec4_ops) {
    Vec4 a = vec4_make(2, 4, 6, 8);
    Vec4 b = vec4_make(1, 2, 3, 4);

    ASSERT(vec4_eq(a + b, vec4_make(3, 6,  9, 12)));
    ASSERT(vec4_eq(a - b, vec4_make(1, 2,  3,  4)));
    ASSERT(vec4_eq(a * b, vec4_make(2, 8, 18, 32)));
    ASSERT(vec4_eq(a / b, vec4_make(2, 2,  2,  2)));
    ASSERT(vec4_eq(a * 2, vec4_make(4, 8, 12, 16)));
}

TEST(test_mat4) {
    Mat4 I = mat4_identity();
    Mat4 T = mat4_translate(vec3_make(1,2,3));

    Mat4 R = mat4_mul(I, T);
    ASSERT(mat4_eq(R, T));

    Mat4 S = mat4_scale(vec3_make(2,2,2));
    Mat4 M = mat4_mul(T, S);

    // spot check scale on diagonal
    ASSERT(feq(M.m[0], 2.0f));
    ASSERT(feq(M.m[5], 2.0f));
    ASSERT(feq(M.m[10], 2.0f));
}

TEST(test_mat4_ops) {
    Mat4 I = mat4_identity();
    Mat4 T = mat4_translate(vec3_make(1,2,3));

    Mat4 R = I * T;
    ASSERT(mat4_eq(R, T));
}

TEST(test_mat4_inverse) {
    Mat4 m = mat4_rotate_x(1.0f);
    Mat4 inv = mat4_inverse_affine(m);
    Mat4 identity = mat4_mul(m, inv);
    
    for(int i = 0; i < 16; i++) {
        ASSERT(feq(identity.m[i], (i % 5 == 0) ? 1.0f : 0.0f));
    }
}

TEST(test_mat4_inverse2) {
    Mat4 T = mat4_translate(vec3_make(3, -2, 5));
    Mat4 R = mat4_rotate_y(1.2f);
    Mat4 S = mat4_scale(vec3_make(2, 3, 4));

    Mat4 M = mat4_mul(T, mat4_mul(R, S));
    Mat4 inv = mat4_inverse_affine(M);

    Mat4 should_be_I = mat4_mul(M, inv);
    ASSERT(mat4_is_identity(should_be_I));
}

TEST(test_mat4_look_at) {
    Vec3 eye    = vec3_make(0,0,0);
    Vec3 target = vec3_make(0,0,-1);
    Vec3 up     = vec3_make(0,1,0);

    Mat4 view = mat4_look_at(eye, target, up);

    Vec3 p = vec3_make(0,0,-5);
    Vec3 v = vec3_transform_point(view, p);

    ASSERT(feq(v.z, -5.0f));
}

TEST(test_mat4_perspective_sanity) {
    Mat4 P = mat4_perspective(1.0f, 1.0f, 0.1f, 100.0f);

    Vec3 p = vec3_make(0,0,-1);
    Vec3 clip = vec3_transform_point(P, p);

    // after projection, z should be in [-1,1] range (OpenGL-style)
    ASSERT(clip.z >= -1.0f && clip.z <= 1.0f);
}

TEST(test_quat) {
    Quat q = quat_identity();
    Mat4 m = quat_to_mat4(q);
    ASSERT(mat4_eq(m, mat4_identity()));
    
    Quat qx = quat_from_axis_angle(vec3_make(1,0,0), (float)M_PI);
    Quat qn = quat_norm(qx);
    float len = sqrtf(qn.x*qn.x + qn.y*qn.y + qn.z*qn.z + qn.w*qn.w);
    ASSERT(feq(len, 1.0f));
}

TEST(test_quat_norm_invariant) {
    Quat q = quat_make(1,2,3,4);
    Quat n = quat_norm(q);

    float len = sqrtf(n.x*n.x + n.y*n.y + n.z*n.z + n.w*n.w);
    ASSERT(feq(len, 1.0f));
}

TEST(test_quat_rotation_preserves_length) {
    Vec3 v = vec3_make(1,2,3);
    float len0 = vec3_length(v);

    Quat q = quat_from_axis_angle(vec3_make(0,1,0), 1.0f);

    Vec3 r = vec3_rotate_quat(v, q);
    float len1 = vec3_length(r);

    ASSERT(feq(len0, len1));
}

TEST(test_quat_slerp_endpoints) {
    Quat a = quat_identity();
    Quat b = quat_from_axis_angle(vec3_make(0,1,0), M_PI);

    Quat r0 = quat_slerp(a, b, 0.0f);
    Quat r1 = quat_slerp(a, b, 1.0f);

    ASSERT(feq(r0.x, a.x) && feq(r0.y, a.y) && feq(r0.z, a.z) && feq(r0.w, a.w));
    ASSERT(feq(r1.x, b.x) && feq(r1.y, b.y) && feq(r1.z, b.z) && feq(r1.w, b.w));
}

TEST(test_transform) {
    Transform a = transform_make(vec3_make(1,0,0),
                            quat_identity(),
                            vec3_make(1,1,1));

    Transform b = transform_make(vec3_make(0,1,0),
                            quat_identity(),
                            vec3_make(1,1,1));

    Transform c = transform_mul(a, b);

    Mat4 ma = transform_to_mat4(a);
    Mat4 mb = transform_to_mat4(b);
    Mat4 mc = transform_to_mat4(c);

    Mat4 expected = mat4_mul(ma, mb);

    ASSERT(mat4_eq(mc, expected));
}
