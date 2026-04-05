#ifndef BASE_H

//
// DECLARATION
//

// Compiler detection

#if defined(_MSC_VER)
    #define COMPILER_MSVC 1
#else
    #define COMPILER_MSVC 0
#endif

#if defined(__clang__)
    #define COMPILER_CLANG 1
#else
    #define COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !COMPILER_CLANG
    #define COMPILER_GCC 1
#else
    #define COMPILER_GCC 0
#endif

// Thread local

#if COMPILER_MSVC
    #define THREAD_LOCAL __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
    #define THREAD_LOCAL __thread
#else
    #error "No thread-local storage support for this compiler"
#endif

// Basic types

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef ptrdiff_t isize;

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFFu
#define U64_MAX 0xFFFFFFFFFFFFFFFFull

// Macro utils

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

#define ALIGN_UP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (MAX((lo), MIN((x), (hi))))

// Asserts

#if COMPILER_MSVC
    #define DEBUG_BREAK() __debugbreak()
#else
    #define DEBUG_BREAK() __builtin_trap()
#endif

#ifndef NDEBUG
    #define ASSERT(x) do { if (!(x)) DEBUG_BREAK(); } while(0)
#else
    #define ASSERT(x)
#endif

#define STATIC_ASSERT(cond, msg) static_assert(cond, msg)

STATIC_ASSERT(sizeof(i8)  == 1, "i8 size incorrect");
STATIC_ASSERT(sizeof(i16) == 2, "i16 size incorrect");
STATIC_ASSERT(sizeof(i32) == 4, "i32 size incorrect");
STATIC_ASSERT(sizeof(i64) == 8, "i64 size incorrect");

STATIC_ASSERT(sizeof(u8)  == 1, "u8 size incorrect");
STATIC_ASSERT(sizeof(u16) == 2, "u16 size incorrect");
STATIC_ASSERT(sizeof(u32) == 4, "u32 size incorrect");
STATIC_ASSERT(sizeof(u64) == 8, "u64 size incorrect");

// Platform detection

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS 1
#else
    #define OS_WINDOWS 0
#endif

#if defined(__linux__)
    #define OS_LINUX 1
#else
    #define OS_LINUX 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define OS_MAC 1
#else
    #define OS_MAC 0
#endif

#if !OS_WINDOWS && !OS_LINUX && !OS_MAC
    #error "Unsupported OS"
#endif

// Memory

void *mem_alloc(u64 size);
void mem_free(void *ptr);

u32 mem_page_size();
u32 mem_granularity();

void *mem_reserve(u64 size);
bool mem_commit(void *ptr, u64 size);
bool mem_decommit(void *ptr, u64 size);
bool mem_release(void *ptr);

// Arena

#define ARENA_BASE_POS ALIGN_UP(sizeof(Arena), ARENA_ALIGN)
#define ARENA_ALIGN (sizeof(void *))

struct Arena {
    u64 reserve_size;
    u64 commit_size;
    u64 pos;
    u64 commit_pos;
};

Arena *arena_create(u64 reserve_size, u64 commit_size);
void arena_free(Arena *a);
void *arena_push(Arena *a, u64 size, bool non_zero);
void arena_pop(Arena *a, u64 size);
void arena_pop_to(Arena *a, u64 pos);
void arena_clear(Arena *a);

#define PUSH_STRUCT(a, T) (T *)arena_push((a), sizeof(T), false)
#define PUSH_STRUCT_NZ(a, T) (T *)arena_push((a), sizeof(T), true)
#define PUSH_ARRAY(a, T, n) (T *)arena_push((a), sizeof(T) * (n), false)
#define PUSH_ARRAY_NZ(a, T, n) (T *)arena_push((a), sizeof(T) * (n), true)

struct Temp_Arena {
    Arena *arena;
    u64 start_pos;
};

Temp_Arena temp_arena_begin(Arena *a);
void temp_arena_end(Temp_Arena temp);

Temp_Arena scratch_arena_get(Arena **conflicts, u32 num_conflicts);
void scratch_arena_release(Temp_Arena scratch);

//
// IMPLEMENTATION
//

#ifdef BASE_IMPLEMENTATION

#include <stdlib.h>

#if OS_WINDOWS
#include <windows.h>
#endif

// Memory

void *mem_alloc(u64 size) {
    return malloc(size);
}

void mem_free(void *ptr) {
    free(ptr);
}

#if OS_WINDOWS

u32 mem_page_size() {
    static u32 result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwPageSize;
    }
    return result;
}

u32 mem_granularity() {
    static u32 result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwAllocationGranularity;
    }
    return result;
}

void *mem_reserve(u64 size) {
    size = ALIGN_UP(size, mem_page_size());
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

bool mem_commit(void *ptr, u64 size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    void *ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(ret == ptr);
    return ret != NULL;
}

bool mem_decommit(void *ptr, u64 size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    return VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

bool mem_release(void *ptr) {
    ASSERT(ptr);
    return VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}

#endif

// Arena

Arena *arena_create(u64 reserve_size, u64 commit_size) {
    u32 page_size = mem_page_size();
    u32 gran = mem_granularity();
    reserve_size = ALIGN_UP(reserve_size, gran);
    commit_size = ALIGN_UP(commit_size, page_size);
    Arena *a = (Arena *)mem_reserve(reserve_size);
    if (!mem_commit(a, commit_size)) return NULL; 
    a->reserve_size = reserve_size;
    a->commit_size = commit_size;
    a->pos = ARENA_BASE_POS;
    a->commit_pos = commit_size;
    return a;
}

void arena_free(Arena *a) {
    mem_release(a);
}

void *arena_push(Arena *a, u64 size, bool non_zero) {
    u64 pos_aligned = ALIGN_UP(a->pos, ARENA_ALIGN);
    u64 new_pos = pos_aligned + size;

    if (new_pos > a->reserve_size) return NULL;

    if (new_pos > a->commit_pos) {
        u64 new_commit_pos = new_pos;
        new_commit_pos += a->commit_size - 1;
        new_commit_pos -= new_commit_pos % a->commit_size;
        new_commit_pos = MIN(new_commit_pos, a->reserve_size);

        u8 *mem = (u8 *)a + a->commit_pos;
        u64 commit_size = new_commit_pos - a->commit_pos;
        if (!mem_commit(mem, commit_size)) return NULL;

        a->commit_pos = new_commit_pos;
    }

    a->pos = new_pos;

    u8 *out = (u8 *)a + pos_aligned;
    if (!non_zero) memset(out, 0, size);
    return out;
}

void arena_pop(Arena *a, u64 size) {
    size = MIN(size, a->pos - ARENA_BASE_POS);
    a->pos -= size;
}

void arena_pop_to(Arena *a, u64 pos) {
    u64 size = pos < a->pos ? a->pos - pos : 0;
    arena_pop(a, size);
}

void arena_clear(Arena *a) {
    arena_pop_to(a, ARENA_BASE_POS);
}

Temp_Arena temp_arena_begin(Arena *a) {
    Temp_Arena temp = {};
    temp.arena = a;
    temp.start_pos = a->pos;
    return temp;
}

void temp_arena_end(Temp_Arena temp) {
    arena_pop_to(temp.arena, temp.start_pos);
}

#define SCRATCH_POOL 2
#define SCRATCH_RESERVE_SIZE (MiB(64))
#define SCRATCH_COMMIT_SIZE (MiB(1))

THREAD_LOCAL static Arena *scratch_pool[SCRATCH_POOL] = { NULL, NULL };

Temp_Arena scratch_arena_get(Arena **conflicts, u32 num_conflicts) {
    i32 scratch_index = -1;

    for (i32 i = 0; i < SCRATCH_POOL; i++) {
        bool found = false;

        for (u32 j = 0; j < num_conflicts; j++) {
            if (scratch_pool[i] == conflicts[j]) {
                found = true;
                break;
            }
        }

        if (!found) {
            scratch_index = i;
            break;
        }
    }

    if (scratch_index == -1) {
        ASSERT(!"No available scratch arena");
        Temp_Arena t = {};
        return t;
    }

    Arena **selected = &scratch_pool[scratch_index];
    if (*selected == NULL) {
        *selected = arena_create(SCRATCH_RESERVE_SIZE, SCRATCH_COMMIT_SIZE);
        ASSERT(*selected);
    }
    return temp_arena_begin(*selected);
}

void scratch_arena_release(Temp_Arena scratch) {
    temp_arena_end(scratch);
}

#endif // BASE_IMPLEMENTATION

#endif // BASE_H
