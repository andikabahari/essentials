#include <stdio.h>

// #define BASE_IMPLEMENTATION
#include "base.h"
#include "example2.h"

int main(void) {
    Arena *perm_arena = arena_create(MiB(100), MiB(1));

    {
        Temp_Arena temp = begin_temp_arena(perm_arena);
        u32 *nums = PUSH_ARRAY(temp.arena, u32, 12);
        end_temp_arena(temp);
    }

    foo();

    Array<int> nums;
    array_init(&nums, 5);
    for (i32 i = 5; i > 0; i--) array_add(&nums, i);
    for (isize i = 0; i < nums.len; i++) printf("Hi #%d\n", nums[i]);

    while (1) {
        arena_push(perm_arena, MiB(1), false);
        getc(stdin);
    }

    arena_destroy(perm_arena);
    return 0;
}
