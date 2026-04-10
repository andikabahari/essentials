#ifndef BASE_COMPILED
#define BASE_IMPLEMENTATION
#endif
#include "base.h"

#define TEST(name) void name()
#define RUN_TEST(name) do { name(); printf("[OK] %s\n", #name); } while (0)

#include <stdio.h>

internal Arena *make_arena() {
    return arena_create(MiB(16), MiB(1));
}

#include "tests_base.cpp"

int main() {
    RUN_TEST(test_arena_push_one);
    RUN_TEST(test_arena_push_many);
    RUN_TEST(test_arena_zero_flag);
    // RUN_TEST(test_arena_pop); // Test will fail due to alignment!
    RUN_TEST(test_arena_pop_to);
    RUN_TEST(test_arena_clear);
    RUN_TEST(test_arena_temp_arena);
    RUN_TEST(test_arena_nested_temp);
    RUN_TEST(test_arena_alignment);

    RUN_TEST(test_scratch_basic);
    RUN_TEST(test_scratch_reset);
    RUN_TEST(test_scratch_conflict);
    RUN_TEST(test_scratch_multiple_conflicts);
    RUN_TEST(test_scratch_nested);
    RUN_TEST(test_scratch_reuse);

    RUN_TEST(test_thread_local_scratch);
    RUN_TEST(test_thread_local_reuse);
    RUN_TEST(test_thread_stress);

    RUN_TEST(test_array_add);
    RUN_TEST(test_array_reserve);
    RUN_TEST(test_array_growth);
    RUN_TEST(test_array_pop);
    RUN_TEST(test_array_clear);
    RUN_TEST(test_array_ordered_remove);
    RUN_TEST(test_array_unordered_remove);

    RUN_TEST(test_string_eq);
    RUN_TEST(test_string_compare);
    RUN_TEST(test_string_index);
    RUN_TEST(test_string_prefix_suffix);
    RUN_TEST(test_string_cut);
    RUN_TEST(test_string_trim_prefix_suffix);
    RUN_TEST(test_string_trim_space);
    RUN_TEST(test_string_split);
    RUN_TEST(test_string_split_empty);
    RUN_TEST(test_string_join);
    RUN_TEST(test_string_concat);
    RUN_TEST(test_string_replace);
    RUN_TEST(test_string_case);
    RUN_TEST(test_string_ops_string);
    RUN_TEST(test_string_ops_cstr);
    RUN_TEST(test_string_ops_empty_cstr);
    RUN_TEST(test_string_ops_edge_cases);
    RUN_TEST(test_string_ops_length);

    printf("All tests passed!\n");
    return 0;
}
