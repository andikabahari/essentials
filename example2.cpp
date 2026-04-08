#include "example2.h"

#include "base.h"
#include "stdio.h"

void foo() {
    printf("example2.cpp\n");

    Array<i32> nums;
    array_init(&nums);
    for (i32 i = 0; i < 5; i++) array_add(&nums, i);
    for (isize i = 0; i < nums.len; i++) printf("Hello #%d\n", nums[i]);
    array_free(&nums);
}
