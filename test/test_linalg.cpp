#define EPS 1e-5f

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

TEST(test_linalg_vec2) {
    Vec2 a = vec2(2, 4);
    Vec2 b = vec2(1, 2);

    ASSERT(vec2_eq(vec2_add(a, b), vec2(3, 6)));
    ASSERT(vec2_eq(vec2_sub(a, b), vec2(1, 2)));
    ASSERT(vec2_eq(vec2_mul(a, b), vec2(2, 8)));
    ASSERT(vec2_eq(vec2_div(a, b), vec2(2, 2)));

    ASSERT(feq(vec2_dot(a, b), 10.0f));
    ASSERT(feq(vec2_length_sq(a), 20.0f));
}

TEST(test_linalg_vec3) {
    Vec3 a = vec3(1, 0, 0);
    Vec3 b = vec3(0, 1, 0);

    ASSERT(vec3_eq(vec3_cross(a, b), vec3(0, 0, 1)));
    ASSERT(feq(vec3_dot(a, b), 0.0f));

    Vec3 c = vec3(2, 0, 0);
    ASSERT(feq(vec3_length(c), 2.0f));
    ASSERT(vec3_eq(vec3_norm(c), vec3(1, 0, 0)));
}

TEST(test_linalg_vec4) {
    Vec4 a = vec4(2, 4, 6, 8);
    Vec4 b = vec4(1, 2, 3, 4);

    ASSERT(vec4_eq(vec4_mul(a, b), vec4(2, 8, 18, 32)));
    ASSERT(vec4_eq(vec4_div(a, b), vec4(2, 2, 2, 2)));

    ASSERT(feq(vec4_dot(a, b), 60.0f));
}

TEST(test_linalg_mat4) {
    Mat4 I = mat4_identity();
    Mat4 T = mat4_translate(vec3(1,2,3));

    Mat4 R = mat4_mul(I, T);
    ASSERT(mat4_eq(R, T));

    Mat4 S = mat4_scale(vec3(2,2,2));
    Mat4 M = mat4_mul(T, S);

    // spot check scale on diagonal
    ASSERT(feq(M.m[0], 2.0f));
    ASSERT(feq(M.m[5], 2.0f));
    ASSERT(feq(M.m[10], 2.0f));
}

TEST(test_linalg_mat4_inverse) {
    Mat4 m = mat4_rotate_x(1.0f);
    Mat4 inv = mat4_inverse_affine(m);
    Mat4 identity = mat4_mul(m, inv);
    
    for(int i = 0; i < 16; i++) {
        ASSERT(feq(identity.m[i], (i % 5 == 0) ? 1.0f : 0.0f));
    }
}

TEST(test_linalg_quat) {
    Quat q = quat_identity();
    Mat4 m = quat_to_mat4(q);
    ASSERT(mat4_eq(m, mat4_identity()));
    
    const float M_PI = 3.14159265358979323846f;
    Quat qx = quat_from_axis_angle(vec3(1,0,0), (float)M_PI);
    Quat qn = quat_norm(qx);
    float len = sqrtf(qn.x*qn.x + qn.y*qn.y + qn.z*qn.z + qn.w*qn.w);
    ASSERT(feq(len, 1.0f));
}

TEST(test_linalg_transform) {
    Transform a = transform(vec3(1,0,0),
                            quat_identity(),
                            vec3(1,1,1));

    Transform b = transform(vec3(0,1,0),
                            quat_identity(),
                            vec3(1,1,1));

    Transform c = transform_mul(a, b);

    Mat4 ma = transform_to_mat4(a);
    Mat4 mb = transform_to_mat4(b);
    Mat4 mc = transform_to_mat4(c);

    Mat4 expected = mat4_mul(ma, mb);

    ASSERT(mat4_eq(mc, expected));
}

