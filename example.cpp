#include <stdio.h>

// #define BASE_IMPLEMENTATION
#include "base.h"

int main(void) {
    Arena *perm_arena = arena_create(MiB(100), MiB(1));

    {
        Temp_Arena temp = temp_arena_begin(perm_arena);
        u32 *nums = PUSH_ARRAY(temp.arena, u32, 12);
        temp_arena_end(temp);
    }

    while (1) {
        arena_push(perm_arena, MiB(1), false);
        getc(stdin);
    }

    arena_free(perm_arena);
    return 0;
}
