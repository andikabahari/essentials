// Arrays

TEST(test_array_add) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_add(&arr, 1);
    array_add(&arr, 2);
    array_add(&arr, 3);

    ASSERT(arr.len == 3);
    ASSERT(arr[0] == 1);
    ASSERT(arr[1] == 2);
    ASSERT(arr[2] == 3);

    arena_destroy(arena);
}

TEST(test_array_reserve) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_reserve(&arr, 100);

    ASSERT(arr.cap >= 100);
    ASSERT(arr.len == 0);

    arena_destroy(arena);
}

TEST(test_array_growth) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, 0, 2, arena);

    array_add(&arr, 1);
    array_add(&arr, 2);
    array_add(&arr, 3); // triggers grow

    ASSERT(arr.len == 3);
    ASSERT(arr[0] == 1);
    ASSERT(arr[1] == 2);
    ASSERT(arr[2] == 3);

    arena_destroy(arena);
}

TEST(test_array_pop) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_add(&arr, 5);
    array_add(&arr, 6);

    int v = array_pop(&arr);

    ASSERT(v == 6);
    ASSERT(arr.len == 1);
    ASSERT(arr[0] == 5);

    arena_destroy(arena);
}

TEST(test_array_clear) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_add(&arr, 1);
    array_add(&arr, 2);

    array_clear(&arr);

    ASSERT(arr.len == 0);

    arena_destroy(arena);
}

TEST(test_array_ordered_remove) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_add(&arr, 1);
    array_add(&arr, 2);
    array_add(&arr, 3);
    array_add(&arr, 4);

    array_ordered_remove(&arr, 1); // remove 2

    ASSERT(arr.len == 3);
    ASSERT(arr[0] == 1);
    ASSERT(arr[1] == 3);
    ASSERT(arr[2] == 4);

    arena_destroy(arena);
}

TEST(test_array_unordered_remove) {
    Arena *arena = make_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_add(&arr, 1);
    array_add(&arr, 2);
    array_add(&arr, 3);
    array_add(&arr, 4);

    array_unordered_remove(&arr, 1); // remove 2

    ASSERT(arr.len == 3);

    // Order not guaranteed
    bool found1 = false, found3 = false, found4 = false;

    for (isize i = 0; i < arr.len; i++) {
        if (arr[i] == 1) found1 = true;
        if (arr[i] == 3) found3 = true;
        if (arr[i] == 4) found4 = true;
    }

    ASSERT(found1 && found3 && found4);

    arena_destroy(arena);
}

// Strings

TEST(test_string_eq) {
    String a = LIT("hello");
    String b = LIT("hello");
    String c = LIT("world");

    ASSERT(string_eq(a,b));
    ASSERT(!string_eq(a,c));
}

TEST(test_string_compare) {
    ASSERT(string_compare(LIT("a"), LIT("b")) < 0);
    ASSERT(string_compare(LIT("b"), LIT("a")) > 0);
    ASSERT(string_compare(LIT("a"), LIT("a")) == 0);
}

TEST(test_string_index) {
    ASSERT(string_index(LIT("hello"), LIT("ll")) == 2);
    ASSERT(string_index(LIT("hello"), LIT("x")) == -1);
}

TEST(test_string_prefix_suffix) {
    ASSERT(string_has_prefix(LIT("foobar"), LIT("foo")));
    ASSERT(string_has_suffix(LIT("foobar"), LIT("bar")));
}

TEST(test_string_cut) {
    String s = LIT("foobar");

    ASSERT(string_eq(string_cut_prefix(s, LIT("foo")), LIT("bar")));
    ASSERT(string_eq(string_cut_suffix(s, LIT("bar")), LIT("foo")));
}

TEST(test_string_trim_prefix_suffix) {
    String s = LIT("foobar");

    ASSERT(string_eq(string_trim_prefix(s, LIT("foo")), LIT("bar")));
    ASSERT(string_eq(string_trim_suffix(s, LIT("bar")), LIT("foo")));

    // no change
    ASSERT(string_eq(string_trim_prefix(s, LIT("xxx")), s));
}

TEST(test_string_trim_space) {
    String s = LIT("  hello \n");

    String t = string_trim_space(s);
    ASSERT(string_eq(t, LIT("hello")));
}

TEST(test_string_split) {
    Arena *arena = make_arena();

    String s = LIT("a,b,c");
    Array<String> parts = string_split(arena, s, LIT(","));

    ASSERT(parts.len == 3);
    ASSERT(string_eq(parts[0], LIT("a")));
    ASSERT(string_eq(parts[1], LIT("b")));
    ASSERT(string_eq(parts[2], LIT("c")));

    arena_destroy(arena);
}

TEST(test_string_split_empty) {
    Arena *arena = make_arena();

    String s = LIT("a,,b");
    Array<String> parts = string_split(arena, s, LIT(","));

    ASSERT(parts.len == 3);
    ASSERT(parts[1].len == 0);

    arena_destroy(arena);
}

TEST(test_string_join) {
    Arena *arena = make_arena();

    Array<String> arr;
    array_init(&arr, arena);

    array_add(&arr, LIT("a"));
    array_add(&arr, LIT("b"));
    array_add(&arr, LIT("c"));

    String s = string_join(arena, arr, LIT(","));

    ASSERT(string_eq(s, LIT("a,b,c")));

    arena_destroy(arena);
}

TEST(test_string_concat) {
    Arena *arena = make_arena();

    String s = string_concat(arena, LIT("foo"), LIT("bar"));

    ASSERT(string_eq(s, LIT("foobar")));

    arena_destroy(arena);
}

TEST(test_string_replace) {
    Arena *arena = make_arena();

    String s = string_replace(arena, LIT("aabb"), LIT("aa"), LIT("x"));

    ASSERT(string_eq(s, LIT("xbb")));

    arena_destroy(arena);
}

TEST(test_string_case) {
    Arena *arena = make_arena();

    ASSERT(string_eq(string_to_lower(arena, LIT("ABC")), LIT("abc")));
    ASSERT(string_eq(string_to_upper(arena, LIT("abc")), LIT("ABC")));

    arena_destroy(arena);
}

TEST(test_string_ops_string) {
    String a = LIT("abc");
    String b = LIT("abc");
    String c = LIT("abd");
    String d = LIT("ab");

    ASSERT(a == b);
    ASSERT(a != c);

    ASSERT(d < a);   // "ab" < "abc"
    ASSERT(c > a);   // "abd" > "abc"

    ASSERT(a <= b);
    ASSERT(a >= b);

    ASSERT(d <= a);
    ASSERT(c >= a);
}

TEST(test_string_ops_cstr) {
    String a = LIT("hello");

    ASSERT(a == "hello");
    ASSERT(a != "world");

    ASSERT(a <  "world");
    ASSERT(a <= "hello");

    ASSERT(a >  "abc");
    ASSERT(a >= "hello");
}

TEST(test_string_ops_empty_cstr) {
    String empty = {0, 0};
    String nonempty = LIT("x");

    ASSERT(empty == "");
    ASSERT(!(empty != ""));

    ASSERT(nonempty != "");
}

TEST(test_string_ops_edge_cases) {
    String a = LIT("a");
    String b = LIT("b");

    ASSERT(a < b);
    ASSERT(!(b < a));

    ASSERT(b > a);
    ASSERT(!(a > b));

    ASSERT(a <= a);
    ASSERT(a >= a);
}

TEST(test_string_ops_length) {
    String short_s = LIT("ab");
    String long_s  = LIT("abc");

    ASSERT(short_s < long_s);
    ASSERT(long_s > short_s);
}
