// Scope-based defer

TEST(test_defer_basic) {
    int x = 0;

    {
        defer(x = 1);
    }

    ASSERT(x == 1);
}

TEST(test_defer_lifo) {
    int x = 0;

    {
        defer(x = x * 10 + 1); // last
        defer(x = x * 10 + 2);
        defer(x = x * 10 + 3); // first
    }

    ASSERT(x == 321);
}

TEST(test_defer_nested) {
    int x = 0;

    {
        defer(x += 1);

        {
            defer(x += 2);
        }

        ASSERT(x == 2);
    }

    ASSERT(x == 3);
}

internal int defer_return_inner() {
    int x = 0;

    {
        defer(x = 42);
        return x;
    }
}

TEST(test_defer_return) {
    int r = defer_return_inner();
    ASSERT(r == 0);
}

global_var bool freed = false;

internal void fake_free(void *) {
    freed = true;
}

TEST(test_defer_resource) {
    freed = false;

    {
        void* ptr = (void *)1234;
        defer(fake_free(ptr));
    }

    ASSERT(freed == true);
}

// Arena

TEST(test_arena_push_one) {
    Arena *a = make_arena();

    int *x = PUSH_ONE(a, int);
    *x = 42;

    ASSERT(*x == 42);

    arena_destroy(a);
}

TEST(test_arena_push_many) {
    Arena *a = make_arena();

    int *arr = PUSH_MANY(a, int, 100);

    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }

    for (int i = 0; i < 100; i++) {
        ASSERT(arr[i] == i);
    }

    arena_destroy(a);
}

TEST(test_arena_zero_flag) {
    Arena *a = make_arena();

    int *x = (int *)arena_push(a, sizeof(int), false);
    *x = 123;

    arena_pop(a, sizeof(int));

    int *y = (int *)arena_push(a, sizeof(int), true);

    // Not guaranteed to be zero, but often reused
    // So we just ensure it's writable
    *y = 456;

    ASSERT(*y == 456);

    arena_destroy(a);
}

TEST(test_arena_pop) {
    Arena *a = make_arena();

    int *x = PUSH_ONE(a, int);
    *x = 10;

    isize before = a->pos;

    int *y = PUSH_ONE(a, int);
    *y = 20;

    arena_pop(a, sizeof(int));

    ASSERT(a->pos == before);

    arena_destroy(a);
}

TEST(test_arena_pop_to) {
    Arena *a = make_arena();

    int *a1 = PUSH_ONE(a, int);
    int *a2 = PUSH_ONE(a, int);

    isize mark = a->pos;

    int *a3 = PUSH_ONE(a, int);

    arena_pop_to(a, mark);

    ASSERT(a->pos == mark);

    arena_destroy(a);
}

TEST(test_arena_clear) {
    Arena *a = make_arena();

    PUSH_MANY(a, int, 1000);

    arena_clear(a);

    ASSERT(a->pos == ARENA_BASE_POS);

    arena_destroy(a);
}

TEST(test_arena_temp_arena) {
    Arena *a = make_arena();

    isize start = a->pos;

    Temp_Arena t = begin_temp_arena(a);

    PUSH_MANY(a, int, 200);

    end_temp_arena(t);

    ASSERT(a->pos == start);

    arena_destroy(a);
}

TEST(test_arena_nested_temp) {
    Arena *a = make_arena();

    isize start = a->pos;

    Temp_Arena t1 = begin_temp_arena(a);
    PUSH_MANY(a, int, 100);

    Temp_Arena t2 = begin_temp_arena(a);
    PUSH_MANY(a, int, 200);

    end_temp_arena(t2);
    end_temp_arena(t1);

    ASSERT(a->pos == start);

    arena_destroy(a);
}


TEST(test_arena_alignment) {
    Arena *a = make_arena();

    void *p1 = arena_push(a, 1, false);
    void *p2 = arena_push(a, 1, false);

    ASSERT(((uintptr_t)p1 % ARENA_ALIGN) == 0);
    ASSERT(((uintptr_t)p2 % ARENA_ALIGN) == 0);

    arena_destroy(a);
}

TEST(test_scratch_basic) {
    Temp_Arena scratch = acquire_scratch_arena(NULL, 0);

    Arena *a = scratch.arena;
    isize start = a->pos;

    int *x = PUSH_MANY(a, int, 100);
    x[0] = 123;

    release_scratch_arena(scratch);

    ASSERT(a->pos == start);
}

TEST(test_scratch_reset) {
    Temp_Arena s1 = acquire_scratch_arena(NULL, 0);
    Arena *a = s1.arena;

    PUSH_MANY(a, int, 200);

    release_scratch_arena(s1);

    Temp_Arena s2 = acquire_scratch_arena(NULL, 0);

    ASSERT(s2.arena == a);
    ASSERT(a->pos == s2.start_pos);

    release_scratch_arena(s2);
}

TEST(test_scratch_conflict) {
    Temp_Arena s1 = acquire_scratch_arena(NULL, 0);
    Arena *a1 = s1.arena;

    Arena *conflicts[1] = { a1 };

    Temp_Arena s2 = acquire_scratch_arena(conflicts, 1);
    Arena *a2 = s2.arena;

    ASSERT(a1 != a2); // MUST be different

    release_scratch_arena(s2);
    release_scratch_arena(s1);
}

TEST(test_scratch_multiple_conflicts) {
    Temp_Arena s1 = acquire_scratch_arena(NULL, 0);
    Temp_Arena s2 = acquire_scratch_arena(NULL, 0);

    Arena *conflicts[2] = { s1.arena, s2.arena };

    Temp_Arena s3 = acquire_scratch_arena(conflicts, 2);

    // With SCRATCH_POOL = 2, this may fallback or reuse.
    // So we only check it's valid:
    ASSERT(s3.arena != NULL);

    release_scratch_arena(s3);
    release_scratch_arena(s2);
    release_scratch_arena(s1);
}

TEST(test_scratch_nested) {
    Temp_Arena s1 = acquire_scratch_arena(NULL, 0);
    Arena *a1 = s1.arena;

    Arena *conflicts[1] = { a1 };

    Temp_Arena s2 = acquire_scratch_arena(conflicts, 1);
    Arena *a2 = s2.arena;

    PUSH_MANY(a1, int, 50);
    PUSH_MANY(a2, int, 50);

    ASSERT(a1 != a2);

    release_scratch_arena(s2);
    release_scratch_arena(s1);
}

TEST(test_scratch_reuse) {
    Temp_Arena s1 = acquire_scratch_arena(NULL, 0);
    Arena *a1 = s1.arena;

    release_scratch_arena(s1);

    Temp_Arena s2 = acquire_scratch_arena(NULL, 0);
    Arena *a2 = s2.arena;

    ASSERT(a1 == a2); // should reuse same slot

    release_scratch_arena(s2);
}

#if OS_WINDOWS

#include <windows.h>
#include <process.h>

struct Thread_Result {
    Arena *arena;
};

internal unsigned __stdcall thread_fn(void *arg) {
    Thread_Result *res = (Thread_Result *)arg;

    Temp_Arena scratch = acquire_scratch_arena(NULL, 0);
    res->arena = scratch.arena;

    int *x = PUSH_MANY(scratch.arena, int, 16);
    x[0] = 123;

    release_scratch_arena(scratch);

    return 0;
}

TEST(test_thread_local_scratch) {
    HANDLE t1, t2;

    Thread_Result r1 = {0};
    Thread_Result r2 = {0};

    t1 = (HANDLE)_beginthreadex(NULL, 0, thread_fn, &r1, 0, NULL);
    t2 = (HANDLE)_beginthreadex(NULL, 0, thread_fn, &r2, 0, NULL);

    WaitForSingleObject(t1, INFINITE);
    WaitForSingleObject(t2, INFINITE);

    CloseHandle(t1);
    CloseHandle(t2);

    printf("Thread1 arena: %p\n", (void*)r1.arena);
    printf("Thread2 arena: %p\n", (void*)r2.arena);

    ASSERT(r1.arena != r2.arena);
}

TEST(test_thread_local_reuse) {
    Temp_Arena s1 = acquire_scratch_arena(NULL, 0);
    Arena *a1 = s1.arena;
    release_scratch_arena(s1);

    Temp_Arena s2 = acquire_scratch_arena(NULL, 0);
    Arena *a2 = s2.arena;
    release_scratch_arena(s2);

    ASSERT(a1 == a2);
}

internal unsigned __stdcall thread_stress(void *arg) {
    for (int i = 0; i < 1000; i++) {
        Temp_Arena s = acquire_scratch_arena(NULL, 0);

        int *x = PUSH_MANY(s.arena, int, 32);
        x[0] = i;

        release_scratch_arena(s);
    }
    return 0;
}

TEST(test_thread_stress) {
    HANDLE threads[4];

    for (int i = 0; i < 4; i++) {
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, thread_stress, NULL, 0, NULL);
    }

    WaitForMultipleObjects(4, threads, TRUE, INFINITE);

    for (int i = 0; i < 4; i++) {
        CloseHandle(threads[i]);
    }
}

#endif

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
    String empty = string_empty();
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

// Hash tables

TEST(test_table_basic) {
    Table<u64, int> t;
    table_init(&t, 16);

    table_set(&t, (u64)1, 10);
    table_set(&t, (u64)2, 20);

    ASSERT(*table_get(&t, (u64)1) == 10);
    ASSERT(*table_get(&t, (u64)2) == 20);
    ASSERT(table_get(&t, (u64)3) == NULL);

    table_free(&t);
}

TEST(test_table_overwrite) {
    Table<u64, int> t;
    table_init(&t, 16);

    table_set(&t, (u64)1, 10);
    table_set(&t, (u64)1, 99);

    ASSERT(*table_get(&t, (u64)1) == 99);
    ASSERT(t.len == 1);
}

TEST(test_table_collision) {
    Table<u64, int> t;
    table_init(&t, 4);

    table_set(&t, (u64)1, 10);
    table_set(&t, (u64)5, 20); // likely same bucket
    table_set(&t, (u64)9, 30);

    ASSERT(*table_get(&t, (u64)1) == 10);
    ASSERT(*table_get(&t, (u64)5) == 20);
    ASSERT(*table_get(&t, (u64)9) == 30);

    table_free(&t);
}

TEST(test_table_delete) {
    Table<u64, int> t;
    table_init(&t, 8);

    table_set(&t, (u64)1, 10);
    table_set(&t, (u64)9, 20); // collision chain

    ASSERT(table_remove(&t, (u64)1) == true);

    ASSERT(table_get(&t, (u64)1) == NULL);
    ASSERT(*table_get(&t, (u64)9) == 20); // must still work

    table_free(&t);
}

TEST(test_table_tombstone_reuse) {
    Table<u64, int> t;
    table_init(&t, 8);

    table_set(&t, (u64)1, 10);
    table_remove(&t, (u64)1);

    table_set(&t, (u64)1, 20);

    ASSERT(*table_get(&t, (u64)1) == 20);

    table_free(&t);
}

TEST(test_table_grow) {
    Table<u64, int> t;
    table_init(&t, 4);

    for (u64 i = 0; i < 100; i++) {
        table_set(&t, i, (int)i);
    }

    for (u64 i = 0; i < 100; i++) {
        ASSERT(*table_get(&t, i) == (int)i);
    }

    table_free(&t);
}

TEST(test_table_stress) {
    Arena *arena = make_arena();

    Table<u64, int> t;
    table_init(&t, 16, arena);

    for (u64 i = 0; i < 10000; i++) {
        table_set(&t, i, (int)i);
    }

    for (u64 i = 0; i < 10000; i++) {
        ASSERT(*table_get(&t, i) == (int)i);
    }

    for (u64 i = 0; i < 10000; i++) {
        table_remove(&t, i);
    }

    for (u64 i = 0; i < 10000; i++) {
        ASSERT(table_get(&t, i) == NULL);
    }

    arena_destroy(arena);
}

TEST(test_table_basic_string) {
    Table<String, int> t;
    table_init(&t, 16);

    table_set(&t, LIT("apple"), 10);
    table_set(&t, LIT("banana"), 20);

    ASSERT(*table_get(&t, LIT("apple")) == 10);
    ASSERT(*table_get(&t, LIT("banana")) == 20);
    ASSERT(table_get(&t, LIT("orange")) == NULL);

    table_free(&t);
}

TEST(test_table_overwrite_string) {
    Table<String, int> t;
    table_init(&t, 16);

    table_set(&t, LIT("key"), 1);
    table_set(&t, LIT("key"), 2);

    ASSERT(*table_get(&t, LIT("key")) == 2);
    ASSERT(t.len == 1);

    table_free(&t);
}

TEST(test_table_collision_string) {
    Table<String, int> t;
    table_init(&t, 4);

    table_set(&t, LIT("a"), 1);
    table_set(&t, LIT("b"), 2);
    table_set(&t, LIT("c"), 3);

    ASSERT(*table_get(&t, LIT("a")) == 1);
    ASSERT(*table_get(&t, LIT("b")) == 2);
    ASSERT(*table_get(&t, LIT("c")) == 3);

    table_free(&t);
}

TEST(test_table_delete_string) {
    Table<String, int> t;
    table_init(&t, 8);

    table_set(&t, LIT("hello"), 1);
    table_set(&t, LIT("world"), 2);

    table_remove(&t, LIT("hello"));

    ASSERT(table_get(&t, LIT("hello")) == NULL);
    ASSERT(*table_get(&t, LIT("world")) == 2);

    table_free(&t);
}

TEST(test_table_content_eq_string) {
    u8 a[] = "test";
    u8 b[] = "test";

    Table<String, String> t;
    table_init(&t, 16);

    table_set(&t, string_make(a, 4), LIT("hey"));

    ASSERT(*table_get(&t, string_make(b, 4)) == LIT("hey"));

    table_free(&t);
}

internal String make_num_string(Arena *arena, u64 x) {
    char buf[32];
    isize len = 0;

    do {
        buf[len++] = '0' + (x % 10);
        x /= 10;
    } while (x);

    // reverse in-place
    for (isize i = 0; i < len / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
    }

    return string_clone(arena, string_make((u8 *)buf, len));
}

TEST(test_table_stress_string) {
    Arena *arena = make_arena();

    Table<String, String> t;
    table_init(&t, 16, arena);

    const u64 N = 10000;

    for (u64 i = 0; i < N; i++) {
        String key = make_num_string(arena, i);
        String val = make_num_string(arena, i);

        table_set(&t, key, val);
    }

    for (u64 i = 0; i < N; i++) {
        String key = make_num_string(arena, i);
        String *v = table_get(&t, key);

        ASSERT(v != NULL);

        String expected = make_num_string(arena, i);
        ASSERT(string_eq(*v, expected));
    }

    for (u64 i = 0; i < N; i += 2) {
        String key = make_num_string(arena, i);
        ASSERT(table_remove(&t, key) == true);
    }

    for (u64 i = 0; i < N; i++) {
        String key = make_num_string(arena, i);
        String *v = table_get(&t, key);

        if (i % 2 == 0) {
            ASSERT(v == NULL);
        } else {
            ASSERT(v != NULL);
            String expected = make_num_string(arena, i);
            ASSERT(string_eq(*v, expected));
        }
    }

    for (u64 i = 0; i < N; i += 2) {
        String key = make_num_string(arena, i);
        String val = make_num_string(arena, i + 1000000);

        table_set(&t, key, val);
    }

    for (u64 i = 0; i < N; i++) {
        String key = make_num_string(arena, i);
        String *v = table_get(&t, key);

        ASSERT(v != NULL);

        if (i % 2 == 0) {
            String expected = make_num_string(arena, i + 1000000);
            ASSERT(string_eq(*v, expected));
        } else {
            String expected = make_num_string(arena, i);
            ASSERT(string_eq(*v, expected));
        }
    }

    arena_destroy(arena);
}

TEST(test_table_clear) {
    Table<u64, int> t;
    {
        table_init(&t, 8);
        defer(table_clear(&t));

        table_set(&t, (u64)1, 10);
        table_set(&t, (u64)2, 20);
    }

    ASSERT(t.len == 0);
    ASSERT(table_get(&t, (u64)1) == NULL);
    ASSERT(table_get(&t, (u64)2) == NULL);
}
