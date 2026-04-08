TEST(test_array_add) {
    Arena *arena = make_test_arena();

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
    Arena *arena = make_test_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_reserve(&arr, 100);

    ASSERT(arr.cap >= 100);
    ASSERT(arr.len == 0);

    arena_destroy(arena);
}

TEST(test_array_growth) {
    Arena *arena = make_test_arena();

    Array<int> arr;
    array_init(&arr, arena, 2);

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
    Arena *arena = make_test_arena();

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
    Arena *arena = make_test_arena();

    Array<int> arr;
    array_init(&arr, arena);

    array_add(&arr, 1);
    array_add(&arr, 2);

    array_clear(&arr);

    ASSERT(arr.len == 0);

    arena_destroy(arena);
}

TEST(test_array_ordered_remove) {
    Arena *arena = make_test_arena();

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
    Arena *arena = make_test_arena();

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
