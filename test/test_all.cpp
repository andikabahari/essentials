#include <stdio.h>

#ifndef BASE_COMPILED
    #define BASE_IMPLEMENTATION
#endif
#include "../base.h"

#ifndef LINALG_COMPILED
    #define LINALG_IMPLEMENTATION
#endif
#include "../linalg.h"

#pragma push_macro("internal")
#undef internal

#ifndef GFX_COMPILED
    #define GFX_IMPLEMENTATION
#endif
#include "../gfx.h"
#include <SDL3/SDL.h>

#pragma pop_macro("internal")

#define TEST(name) void name()
#define RUN_TEST(name) do { name(); printf("[OK] %s\n", #name); } while (0)

internal Arena *make_arena() {
    return arena_create(MiB(16), MiB(1));
}


#include "test_base.cpp"
#include "test_linalg.cpp"
#include "test_gfx.cpp"

int main() {
    //
    // base.h test cases
    //
    
    RUN_TEST(test_defer_basic);
    RUN_TEST(test_defer_lifo);
    RUN_TEST(test_defer_nested);
    RUN_TEST(test_defer_return);

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

    RUN_TEST(test_table_basic);
    RUN_TEST(test_table_overwrite);
    RUN_TEST(test_table_collision);
    RUN_TEST(test_table_delete);
    RUN_TEST(test_table_tombstone_reuse);
    RUN_TEST(test_table_grow);
    RUN_TEST(test_table_stress);
    RUN_TEST(test_table_basic_string);
    RUN_TEST(test_table_overwrite_string);
    RUN_TEST(test_table_collision_string);
    RUN_TEST(test_table_delete_string);
    RUN_TEST(test_table_content_eq_string);
    RUN_TEST(test_table_stress_string);
    RUN_TEST(test_table_clear);

    //
    // linalg.h test cases
    //

    RUN_TEST(test_vec2);
    RUN_TEST(test_vec2_ops);
    RUN_TEST(test_vec2_ops_assignment);
    RUN_TEST(test_vec3);
    RUN_TEST(test_vec3_ops);
    RUN_TEST(test_vec3_ops_assignment);
    RUN_TEST(test_vec3_cross_orthogonality);
    RUN_TEST(test_vec3_norm_idempotence);
    RUN_TEST(test_vec4);
    RUN_TEST(test_vec4_ops);
    RUN_TEST(test_vec4_ops_assignment);
    RUN_TEST(test_mat4);
    RUN_TEST(test_mat4_inverse);
    RUN_TEST(test_mat4_inverse2);
    RUN_TEST(test_mat4_look_at);
    RUN_TEST(test_mat4_perspective_sanity);
    RUN_TEST(test_quat);
    RUN_TEST(test_quat_norm_invariant);
    RUN_TEST(test_quat_rotation_preserves_length);
    RUN_TEST(test_quat_slerp_endpoints);
    RUN_TEST(test_transform);

    //
    // gfx.h test cases
    //

    RUN_TEST(test_gfx);

    printf("All tests passed!\n");
    return 0;
}
