#include "base.h"

#define TEST(name) void name()
#define RUN_TEST(name) do { name(); printf("[OK] %s\n", #name); } while (0)

#include <stdio.h>

internal Arena *make_test_arena() {
    return arena_create(MiB(16), MiB(1));
}

#include "tests_base.cpp"

int main() {
    RUN_TEST(test_array_add);
    RUN_TEST(test_array_reserve);
    RUN_TEST(test_array_growth);
    RUN_TEST(test_array_pop);
    RUN_TEST(test_array_clear);
    RUN_TEST(test_array_ordered_remove);
    RUN_TEST(test_array_unordered_remove);

    printf("All tests passed!\n");
    return 0;
}
