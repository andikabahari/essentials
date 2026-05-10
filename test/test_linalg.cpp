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

inline bool quat_eq(Quat a, Quat b) {
    return feq(a.x, b.x)
        && feq(a.y, b.y)
        && feq(a.z, b.z)
        && feq(a.w, b.w);
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

    ASSERT(vec2_eq(a + b, vec2_make( 3,  6)));
    ASSERT(vec2_eq(a - b, vec2_make( 1,  2)));
    ASSERT(vec2_eq(  - a, vec2_make(-2, -4)));
    ASSERT(vec2_eq(a * b, vec2_make( 2,  8)));
    ASSERT(vec2_eq(a / b, vec2_make( 2,  2)));
    ASSERT(vec2_eq(a * 2, vec2_make( 4,  8)));
    ASSERT(vec2_eq(2 * a, vec2_make( 4,  8)));
    ASSERT(vec2_eq(a / 2, vec2_make( 1,  2)));
}

TEST(test_vec2_ops_assignment) {
    Vec2 a = vec2_make(2, 4);
    Vec2 b = vec2_make(1, 2);

    Vec2 x = a;
    x += b;
    ASSERT(vec2_eq(x, vec2_make(3, 6)));

    x = a;
    x -= b;
    ASSERT(vec2_eq(x, vec2_make(1, 2)));

    x = a;
    x *= b;
    ASSERT(vec2_eq(x, vec2_make(2, 8)));

    x = a;
    x *= 2;
    ASSERT(vec2_eq(x, vec2_make(4, 8)));

    x = a;
    x /= b;
    ASSERT(vec2_eq(x, vec2_make(2, 2)));

    x = a;
    x /= 2;
    ASSERT(vec2_eq(x, vec2_make(1, 2)));
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

    ASSERT(vec3_eq(a + b, vec3_make( 3,  6,  9)));
    ASSERT(vec3_eq(a - b, vec3_make( 1,  2,  3)));
    ASSERT(vec3_eq(  - a, vec3_make(-2, -4, -6)));
    ASSERT(vec3_eq(a * b, vec3_make( 2,  8, 18)));
    ASSERT(vec3_eq(a / b, vec3_make( 2,  2,  2)));
    ASSERT(vec3_eq(a * 2, vec3_make( 4,  8, 12)));
    ASSERT(vec3_eq(2 * a, vec3_make( 4,  8, 12)));
    ASSERT(vec3_eq(a / 2, vec3_make( 1,  2,  3)));
}

TEST(test_vec3_ops_assignment) {
    Vec3 a = vec3_make(2, 4, 6);
    Vec3 b = vec3_make(1, 2, 3);

    Vec3 x = a;
    x += b;
    ASSERT(vec3_eq(x, vec3_make(3, 6, 9)));

    x = a;
    x -= b;
    ASSERT(vec3_eq(x, vec3_make(1, 2, 3)));

    x = a;
    x *= b;
    ASSERT(vec3_eq(x, vec3_make(2, 8, 18)));

    x = a;
    x *= 2;
    ASSERT(vec3_eq(x, vec3_make(4, 8, 12)));

    x = a;
    x /= b;
    ASSERT(vec3_eq(x, vec3_make(2, 2, 2)));

    x = a;
    x /= 2;
    ASSERT(vec3_eq(x, vec3_make(1, 2, 3)));
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

    ASSERT(vec4_eq(a + b, vec4_make( 3,  6,  9, 12)));
    ASSERT(vec4_eq(a - b, vec4_make( 1,  2,  3,  4)));
    ASSERT(vec4_eq(  - a, vec4_make(-2, -4, -6, -8)));
    ASSERT(vec4_eq(a * b, vec4_make( 2,  8, 18, 32)));
    ASSERT(vec4_eq(a / b, vec4_make( 2,  2,  2,  2)));
    ASSERT(vec4_eq(a * 2, vec4_make( 4,  8, 12, 16)));
    ASSERT(vec4_eq(2 * a, vec4_make( 4,  8, 12, 16)));
    ASSERT(vec4_eq(a / 2, vec4_make( 1,  2,  3,  4)));
}

TEST(test_vec4_ops_assignment) {
    Vec4 a = vec4_make(2, 4, 6, 8);
    Vec4 b = vec4_make(1, 2, 3, 4);

    Vec4 x = a;
    x += b;
    ASSERT(vec4_eq(x, vec4_make(3, 6, 9, 12)));

    x = a;
    x -= b;
    ASSERT(vec4_eq(x, vec4_make(1, 2, 3, 4)));

    x = a;
    x *= b;
    ASSERT(vec4_eq(x, vec4_make(2, 8, 18, 32)));

    x = a;
    x *= 2;
    ASSERT(vec4_eq(x, vec4_make(4, 8, 12, 16)));

    x = a;
    x /= b;
    ASSERT(vec4_eq(x, vec4_make(2, 2, 2, 2)));

    x = a;
    x /= 2;
    ASSERT(vec4_eq(x, vec4_make(1, 2, 3, 4)));
}

TEST(test_mat4) {
    Mat4 I = mat4_identity();
    Mat4 T = mat4_translate(vec3_make(1,2,3));

    Mat4 R = mat4_mul(I, T);
    ASSERT(mat4_eq(R, T));

    Mat4 S = mat4_scaling(vec3_make(2,2,2));
    Mat4 M = mat4_mul(T, S);

    // spot check scale on diagonal
    ASSERT(feq(M.m[0], 2.0f));
    ASSERT(feq(M.m[5], 2.0f));
    ASSERT(feq(M.m[10], 2.0f));
}

TEST(test_mat4_ops) {
    Mat4 a = {{
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
    }};

    Mat4 b = {{
        16, 15, 14, 13,
        12, 11, 10,  9,
         8,  7,  6,  5,
         4,  3,  2,  1,
    }};

    ASSERT(mat4_eq(a + b, {{
        17, 17, 17, 17,
        17, 17, 17, 17,
        17, 17, 17, 17,
        17, 17, 17, 17,
    }}));

    ASSERT(mat4_eq(a - b, {{
       -15, -13, -11,  -9,
        -7,  -5,  -3,  -1,
         1,   3,   5,   7,
         9,  11,  13,  15,
    }}));

    ASSERT(mat4_eq(-a, {{
        -1,  -2,  -3,  -4,
        -5,  -6,  -7,  -8,
        -9, -10, -11, -12,
       -13, -14, -15, -16,
    }}));

    ASSERT(mat4_eq(a * 2.0f, {{
         2,  4,  6,  8,
        10, 12, 14, 16,
        18, 20, 22, 24,
        26, 28, 30, 32,
    }}));

    ASSERT(mat4_eq(2.0f * a, {{
         2,  4,  6,  8,
        10, 12, 14, 16,
        18, 20, 22, 24,
        26, 28, 30, 32,
    }}));

    ASSERT(mat4_eq(a / 2.0f, {{
        0.5f, 1.0f, 1.5f, 2.0f,
        2.5f, 3.0f, 3.5f, 4.0f,
        4.5f, 5.0f, 5.5f, 6.0f,
        6.5f, 7.0f, 7.5f, 8.0f,
    }}));

    /* mat4 x vec4 */ {
        Mat4 t = mat4_translate(vec3_make(3, 4, 5));

        Vec4 v = vec4_make(1, 2, 3, 1);

        ASSERT(vec4_eq(
            t * v,
            vec4_make(4, 6, 8, 1)
        ));
    }

    /* mat4 x mat4 */ {
        Mat4 s = mat4_scaling(vec3_make(2, 2, 2));
        Mat4 t = mat4_translate(vec3_make(1, 2, 3));

        Mat4 m = t * s;

        Vec4 v = vec4_make(1, 1, 1, 1);

        ASSERT(vec4_eq(
            m * v,
            vec4_make(3, 4, 5, 1)
        ));
    }
}

TEST(test_mat4_ops_assignment) {
    Mat4 a = {{
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
    }};

    Mat4 b = {{
        16, 15, 14, 13,
        12, 11, 10,  9,
         8,  7,  6,  5,
         4,  3,  2,  1,
    }};

    Mat4 x = a;

    x += b;
    ASSERT(mat4_eq(x, {{
        17, 17, 17, 17,
        17, 17, 17, 17,
        17, 17, 17, 17,
        17, 17, 17, 17,
    }}));

    x = a;
    x -= b;
    ASSERT(mat4_eq(x, {{
       -15, -13, -11,  -9,
        -7,  -5,  -3,  -1,
         1,   3,   5,   7,
         9,  11,  13,  15,
    }}));

    x = a;
    x *= 2.0f;
    ASSERT(mat4_eq(x, {{
         2,  4,  6,  8,
        10, 12, 14, 16,
        18, 20, 22, 24,
        26, 28, 30, 32,
    }}));

    x = a;
    x /= 2.0f;
    ASSERT(mat4_eq(x, {{
        0.5f, 1.0f, 1.5f, 2.0f,
        2.5f, 3.0f, 3.5f, 4.0f,
        4.5f, 5.0f, 5.5f, 6.0f,
        6.5f, 7.0f, 7.5f, 8.0f,
    }}));

    x = mat4_identity();
    x *= mat4_scaling(vec3_make(2, 3, 4));

    ASSERT(vec4_eq(
        x * vec4_make(1, 1, 1, 1),
        vec4_make(2, 3, 4, 1)
    ));
}

TEST(test_mat4_inverse) {
    Mat4 m = mat4_rotate(1.0f, vec3_make(1,0,0));
    Mat4 inv = mat4_inverse(m);
    Mat4 identity = mat4_mul(m, inv);
    
    for(int i = 0; i < 16; i++) {
        ASSERT(feq(identity.m[i], (i % 5 == 0) ? 1.0f : 0.0f));
    }
}

TEST(test_mat4_inverse2) {
    Mat4 T = mat4_translate(vec3_make(3, -2, 5));
    Mat4 R = mat4_rotate(1.2f, vec3_make(0,1,0));
    Mat4 S = mat4_scaling(vec3_make(2, 3, 4));

    Mat4 M = mat4_mul(T, mat4_mul(R, S));
    Mat4 inv = mat4_inverse(M);

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

TEST(test_quat_ops) {
    Quat a = quat_make(1, 2, 3, 4);
    Quat b = quat_make(5, 6, 7, 8);

    ASSERT(quat_eq(a + b, quat_make( 6,  8, 10, 12)));
    ASSERT(quat_eq(a - b, quat_make(-4, -4, -4, -4)));
    ASSERT(quat_eq(   -a, quat_make(-1, -2, -3, -4)));

    ASSERT(quat_eq(a * 2.0f,
        quat_make(2, 4, 6, 8)));

    ASSERT(quat_eq(2.0f * a,
        quat_make(2, 4, 6, 8)));

    ASSERT(quat_eq(a / 2.0f,
        quat_make(0.5f, 1.0f, 1.5f, 2.0f)));

    /* quat x vec3 */ {
        Quat q = quat_from_axis_angle(
            vec3_make(0, 0, 1),
            M_PI * 0.5f
        );

        Vec3 v = vec3_make(1, 0, 0);

        Vec3 r = q * v;

        ASSERT(feq(r.x, 0.0f));
        ASSERT(feq(r.y, 1.0f));
        ASSERT(feq(r.z, 0.0f));
    }

    /* quat x quat */ {
        Quat q1 = quat_from_axis_angle(
            vec3_make(0, 0, 1),
            M_PI * 0.5f
        );

        Quat q2 = quat_from_axis_angle(
            vec3_make(0, 0, 1),
            M_PI * 0.5f
        );

        Quat q = q1 * q2;

        Vec3 v = vec3_make(1, 0, 0);
        Vec3 r = q * v;

        ASSERT(feq(r.x, -1.0f));
        ASSERT(feq(r.y,  0.0f));
        ASSERT(feq(r.z,  0.0f));
    }
}

TEST(test_quat_ops_assignment) {
    Quat a = quat_make(1, 2, 3, 4);
    Quat b = quat_make(5, 6, 7, 8);

    Quat x = a;

    x += b;
    ASSERT(quat_eq(x, quat_make( 6,  8, 10, 12)));

    x = a;
    x -= b;
    ASSERT(quat_eq(x, quat_make(-4, -4, -4, -4)));

    x = a;
    x *= 2.0f;
    ASSERT(quat_eq(x, quat_make(2, 4, 6, 8)));

    x = a;
    x /= 2.0f;
    ASSERT(quat_eq(x,
        quat_make(0.5f, 1.0f, 1.5f, 2.0f)));

    x = quat_identity();

    x *= quat_from_axis_angle(
        vec3_make(0, 0, 1),
        M_PI * 0.5f
    );

    Vec3 r = x * vec3_make(1, 0, 0);

    ASSERT(feq(r.x, 0.0f));
    ASSERT(feq(r.y, 1.0f));
    ASSERT(feq(r.z, 0.0f));
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
