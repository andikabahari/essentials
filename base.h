//
// base.h
//
// This is a public domain C/C++ library.
// No warranty implied, use at your own risk!
//
// This library contains things that I wish C provided by default.
// Those things are basic data structures (arrays, strings, hash tables).
// The first time I tried C, I liked how simple it is. It's so simple
// that it tastes bland and raw, so I want to add some flavors to
// satisfy my tongue.
//
// You can find most of the things this library has in the C++
// standard template library (STL). But if you're just like me, if you
// want to use C with a little bit of flavors--templates,
// overloading--if you want to keep everything explicit init-and-free
// (no RAII), maybe this library will be helpful for you (hopefully).
//
// EXAMPLE USAGE
//
//   Arenas
//
//     Create an arena with reserve size of n bytes and commit size of m bytes:
//       Arena *arena = arena_create(n, m);
//
//     Push n bytes onto the arena:
//       void *ptr = arena_push(arena, n);
//
//     Push one sizeof(T) bytes onto the arena:
//       void *ptr = arena_push_type(arena, T);
//
//     Push n sizeof(T) bytes onto the arena:
//       void *ptr = arena_push_array(arena, T, n);
//
//     Clear the arena:
//       arena_clear(arena);
//
//     Destroy the arena:
//       arena_destroy(arena);
//
//     Per-thread scratch arena for temporary allocation:
//       Arena_Temp temp = arena_begin_scratch(NULL, 0);
//       arena_push_array(temp.arena, u8, 100);
//       arena_end_scratch(temp);
//
//   Allocators
//
//     Allocator struct (heap and arena):
//       Allocator ah = heap_allocator();
//       Allocator aa = arena_allocator(arena);
//
//     Using allocator:
//       isize sz1 = 69;
//       isize sz2 = 420;
//       void *ptr = allocator_alloc(ah, sz1);
//       ptr = allocator_resize(ah, ptr, sz1, sz2);
//       allocator_free(ah, ptr);
//
//       // Shortcut macros
//       // heap_alloc(sz);
//       // heap_resize(ptr, oldsz, newsz);
//       // heap_free(ptr);
//
//   Logs
//
//     log_printf(LOG_INFO, "It says... %s", "hello!");
//
//     // Shortcut macros
//     // log_info(fmt, ...);
//     // log_warning(fmt, ...);
//     // log_error(fmt, ...);
//
//   Arrays
//
//     Init an array of integers:
//       Array<int> nums;
//       array_init(&nums, heap_allocator());
//
//     Add items to the array:
//       array_add(&nums, 1);
//       array_add(&nums, 2);
//       array_add(&nums, 3);
//
//     Access the i'th item in the array:
//       nums[i]
//
//     Get length of the array:
//       nums.len
//
//     Get capacity of the array:
//       nums.cap
//
//     Free the array:
//       array_free(&nums);
//
//   Strings
//
//     Make a string with LIT(s):
//       String text = LIT("Foo");
//
//     Access the i'th byte in the string:
//       text[i]
//
//     Get length of the string:
//       text.len
//
//     Compare two strings:
//       text == "Foo"
//       text == LIT("Foo")
//
//     Printf-style arguments:
//       printf("%.*s", FMT(text));
//
//     Concatenate two strings:
//       String new_text = string_concat(heap_allocator(), text, LIT("Bar"));
//
//     Convert to C string:
//       const char *cstr = string_to_cstr(heap_allocator(), text);
//
//   Hash Tables
//
//     Init a hash table of type Table<String, int>:
//       Table<String, int> height;
//       table_init(&height, heap_allocator());
//
//     Set an item to the hash table:
//       table_set(&height, LIT("Asep"), 192);
//       table_set(&height, LIT("Udin"), 210);
//
//     Get an item from the hash table:
//       int *x = table_get(&height, LIT("Asep"));
//
//     Free the table:
//       table_free(&height);
// 

#ifndef BASE_H
#define BASE_H

// Foreign includes

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// OS detection

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

// Language detection

#define C_23 202311L
#define C_17 201710L
#define C_11 201112L
#define C_99 199901L
#define C_95 199409L
#define C_89 0L

#define CPP_26 202600L
#define CPP_23 202302L
#define CPP_20 202002L
#define CPP_17 201703L
#define CPP_14 201402L
#define CPP_11 201103L
#define CPP_98 199711L

#if defined(__cplusplus)
    #define LANG_CPP    1
    #define LANG_C      0
    #if defined(_MSVC_LANG)
        #define CPP_VERSION _MSVC_LANG
    #else
        #define CPP_VERSION __cplusplus
    #endif
    #define C_VERSION   0L
#elif defined(__STDC__) || defined(__STDC_VERSION__) || defined(_MSC_VER)
    #define LANG_CPP    0
    #define LANG_C      1
    #define CPP_VERSION 0L
    #if defined(__STDC_VERSION__)
        #define C_VERSION __STDC_VERSION__
    #elif defined(_MSC_VER) && defined(_MSVC_LANG)
        #define C_VERSION _MSVC_LANG 
    #else
        #define C_VERSION C_89
    #endif
#else
    #define LANG_CPP    0
    #define LANG_C      0
    #define C_VERSION   0L
    #define CPP_VERSION 0L
#endif

#if LANG_C && C_VERSION < C_11
    #pragma message("Warning: C11 or later recommended")
#elif LANG_CPP && CPP_VERSION < CPP_11
    #pragma message("Warning: C++11 or later recommended")
#endif

// Debug option

#if !defined(BUILD_DEBUG)
    #define BUILD_DEBUG 1
#endif

// C linkage

#if LANG_CPP
    #define C_LINKAGE_BEGIN extern "C" {
    #define C_LINKAGE_END   }
    #define C_LINKAGE       extern "C"
#else
    #define C_LINKAGE_BEGIN
    #define C_LINKAGE_END
    #define C_LINKAGE
#endif

// Thread local

#if CPP_VERSION >= CPP_11
    #define THREAD_LOCAL thread_local
#elif C_VERSION >= C_11
    #define THREAD_LOCAL _Thread_local
#elif COMPILER_MSVC
    #define THREAD_LOCAL __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
    #define THREAD_LOCAL __thread
#else
    #error "No thread-local storage support available."
#endif

// Printf format

#if COMPILER_CLANG || COMPILER_GCC
    #define PRINTF_FORMAT(fmt_index, first_arg)\
        __attribute__((format(printf, fmt_index, first_arg)))
#else
    #define PRINTF_FORMAT(fmt_index, first_arg)
#endif

// Asserts

#if COMPILER_MSVC
    #define TRAP() __debugbreak()
#else
    #define TRAP() __builtin_trap()
#endif

#if BUILD_DEBUG
    #define ASSERT(expr) ASSERT_MSG(expr, NULL)
    #define ASSERT_MSG(expr, fmt, ...)\
        do {\
            if (!(expr)) {\
                print_assert_message("Assertion failed", #expr, __FILE__, (int)__LINE__, fmt, ##__VA_ARGS__);\
                TRAP();\
            }\
        } while (0)
    #define PANIC(fmt, ...)\
        do {\
            print_assert_message("Panic", NULL, __FILE__, (int)__LINE__, fmt, ##__VA_ARGS__);\
            TRAP();\
        } while (0)
#else
    #define ASSERT(expr)               (expr)
    #define ASSERT_MSG(expr, fmt, ...) (expr)
    #define PANIC(fmt, ...)
#endif

static inline void print_assert_message(const char *prefix, const char *expr, const char *file, int line, const char *fmt, ...) {
    fprintf(stderr, "%s(%d): %s: ", file, line, prefix);

    if (expr) {
        fprintf(stderr, "`%s` ", expr);
    }

    if (fmt) {
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }

    fprintf(stderr, "\n");
}

#if CPP_VERSION >= CPP_11
    #define STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#elif C_VERSION >= C_11
    #define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)
#else
    #define STATIC_ASSERT(expr, msg) \
        typedef char static_assertion_##__LINE__[(expr) ? 1 : -1]
#endif

// Bit manipulation

#define BIT(n)            (1ULL << (n))
#define BIT_SET(x, n)     ((x) |=  BIT(n))
#define BIT_CLEAR(x, n)   ((x) &= ~BIT(n))
#define BIT_TOGGLE(x, n)  ((x) ^=  BIT(n))
#define BIT_CHECK(x, n)   (((x) >> (n)) & 1)

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

// Math helpers

#define ALIGN_UP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (MAX((lo), MIN((x), (hi))))

#define ABS(x)        ((x) < 0 ? -(x) : (x))
#define SIGN(x)       ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#define SQUARE(x)     ((x) * (x))
#define IS_POW2(x)    ((x) != 0 && ((x) & ((x) - 1)) == 0)
#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))

// Stringify / concat

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x)       STRINGIFY_INNER(x)

#define CONCAT_INNER(a, b) a##b
#define CONCAT(a, b)       CONCAT_INNER(a, b)

// Misc

#define UNUSED(x)      ((void)(x))
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))
#define SWAP(T, a, b)  do { T _swap_tmp_ = (a); (a) = (b); (b) = _swap_tmp_; } while (0)

#define for_count(it, stop)                 for_range(it, 0, (stop))
#define for_range(it, start, stop)          for_range_ex(it, (start), (stop), 1)
#define for_range_ex(it, start, stop, step) for (isize it = (start); it < (stop); it += (step))

// Scope-based defer
// https://github.com/gingerBill/gb/blob/master/gb.h

#if LANG_CPP

template <typename T> struct gbRemoveReference       { typedef T Type; };
template <typename T> struct gbRemoveReference<T &>  { typedef T Type; };
template <typename T> struct gbRemoveReference<T &&> { typedef T Type; };

template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &t)  { return static_cast<T &&>(t); }
template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &&t) { return static_cast<T &&>(t); }
template <typename T> inline T &&gb_move   (T &&t)                                   { return static_cast<typename gbRemoveReference<T>::Type &&>(t); }
template <typename F>
struct gbprivDefer {
    F f;
    gbprivDefer(F &&f) : f(gb_forward<F>(f)) {}
    ~gbprivDefer() { f(); }
};
template <typename F> gbprivDefer<F> gb__defer_func(F &&f) { return gbprivDefer<F>(gb_forward<F>(f)); }

#define defer(code) auto CONCAT(defer_, __LINE__) = gb__defer_func([&]()->void{code;})

#endif

// Basic types

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef float     f32;
typedef double    f64;

typedef size_t    usize;
typedef ptrdiff_t isize;

STATIC_ASSERT(sizeof(i8)  == 1, "i8 size incorrect");
STATIC_ASSERT(sizeof(i16) == 2, "i16 size incorrect");
STATIC_ASSERT(sizeof(i32) == 4, "i32 size incorrect");
STATIC_ASSERT(sizeof(i64) == 8, "i64 size incorrect");

STATIC_ASSERT(sizeof(u8)  == 1, "u8 size incorrect");
STATIC_ASSERT(sizeof(u16) == 2, "u16 size incorrect");
STATIC_ASSERT(sizeof(u32) == 4, "u32 size incorrect");
STATIC_ASSERT(sizeof(u64) == 8, "u64 size incorrect");

STATIC_ASSERT(sizeof(usize) == sizeof(isize), "usize and isize do not equal");

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFFu
#define U64_MAX 0xFFFFFFFFFFFFFFFFull

#define I8_MIN  ((i8) 0x80)
#define I8_MAX  ((i8) 0x7F)
#define I16_MIN ((i16)0x8000)
#define I16_MAX ((i16)0x7FFF)
#define I32_MIN ((i32)0x80000000)
#define I32_MAX ((i32)0x7FFFFFFF)
#define I64_MIN ((i64)0x8000000000000000)
#define I64_MAX ((i64)0x7FFFFFFFFFFFFFFF)

#define F32_MIN     (1.17549435e-38f)
#define F32_MAX     (3.40282347e+38f)
#define F32_EPSILON (1.19209290e-07f)
#define F32_PI      (3.1415927f)
#define F64_MIN     (2.2250738585072014e-308)
#define F64_MAX     (1.7976931348623157e+308)
#define F64_EPSILON (2.2204460492503131e-16)
#define F64_PI      (3.141592653589793)

C_LINKAGE_BEGIN

// Memory

#define DEFAULT_MEMORY_ALIGNMENT (2 * sizeof(void *))

typedef void *Alloc_Proc (size_t sz);
typedef void *Resize_Proc(void *ptr, size_t newsz);
typedef void  Free_Proc  (void *ptr);

void mem_set_procs(Alloc_Proc *alloc_proc, Resize_Proc *resize_proc, Free_Proc *free_proc);

void *mem_alloc(isize sz);
void *mem_resize(void *ptr, isize newsz);
void  mem_free(void *ptr);

#define mem_copy    memcpy
#define mem_move    memmove
#define mem_set     memset
#define mem_compare memcmp
#define mem_char    memchr

#define mem_zero(x)       mem_set(&(x), 0, sizeof((x)))
#define mem_zero_ptr(x)   mem_set((x), 0, sizeof(*(x)))
#define mem_zero_array(x) mem_set((x), 0, sizeof((x)))

// Arena

#define ARENA_BASE_POS ALIGN_UP(sizeof(Arena), ARENA_ALIGN)
#define ARENA_ALIGN    (sizeof(void *))

typedef struct {
    isize reserve_size;
    isize commit_size;
    isize pos;
    isize commit_pos;
} Arena;

Arena *arena_create(isize reserve_size, isize commit_size);
void   arena_destroy(Arena *a);
void  *arena_push(Arena *a, isize size, bool non_zero);
void   arena_pop(Arena *a, isize size);
void   arena_pop_to(Arena *a, isize pos);
void   arena_clear(Arena *a);

#define arena_push_type(a, T)        (T *)arena_push((a), sizeof(T), false)
#define arena_push_type_nz(a, T)     (T *)arena_push((a), sizeof(T), true)
#define arena_push_array(a, T, n)    (T *)arena_push((a), sizeof(T) * (n), false)
#define arena_push_array_nz(a, T, n) (T *)arena_push((a), sizeof(T) * (n), true)

typedef struct {
    Arena *arena;
    isize start_pos;
} Arena_Temp;

Arena_Temp arena_begin_temp(Arena *a);
void       arena_end_temp(Arena_Temp temp);

// These are default arbitary values, you can define these as you wish.
#ifndef ARENA_SCRATCH_POOL
    #define ARENA_SCRATCH_POOL         2
#endif
#ifndef ARENA_SCRATCH_RESERVE_SIZE
    #define ARENA_SCRATCH_RESERVE_SIZE MiB(32)
#endif
#ifndef ARENA_SCRATCH_COMMIT_SIZE
    #define ARENA_SCRATCH_COMMIT_SIZE  MiB(4)
#endif

Arena_Temp arena_begin_scratch(Arena **conflicts, i32 num_conflicts);
void       arena_end_scratch(Arena_Temp scratch);

// Custom allocation

typedef enum {
    ALLOCATION_ALLOC,
    ALLOCATION_RESIZE,
    ALLOCATION_FREE,
    ALLOCATION_FREE_ALL,
} Allocation_Mode;

#define ALLOCATOR_PROC(name)\
    void *name(void *alloc_data, Allocation_Mode alloc_mode,\
               isize newsz, isize alignment,\
               void *oldmem, isize oldsz)

typedef ALLOCATOR_PROC(Allocator_Proc);

typedef struct {
    Allocator_Proc *proc;
    void *data;
} Allocator;

void *allocator_alloc(Allocator a, isize sz);
void *allocator_resize(Allocator a, void *ptr, isize oldsz, isize newsz);
void  allocator_free(Allocator a, void *ptr);
void  allocator_free_all(Allocator a);

#define alloc_struct(a, T)   (T *)allocator_alloc((a), sizeof(T))
#define alloc_array(a, T, n) (T *)allocator_alloc((a), (n) * sizeof(T))

Allocator heap_allocator(void);
ALLOCATOR_PROC(heap_allocator_proc);

#define heap_alloc(sz)                 allocator_alloc(heap_allocator(), (sz))
#define heap_resize(ptr, oldsz, newsz) allocator_resize(heap_allocator(), (ptr), (oldsz), (newsz))
#define heap_free(ptr)                 allocator_free(heap_allocator(), (ptr))

Allocator arena_allocator(Arena *arena);
ALLOCATOR_PROC(arena_allocator_proc);

// Logs

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
} Log_Level;

#define LOG_PROC(name)\
    void name(Log_Level level, const char *fmt, va_list args)

typedef LOG_PROC(Log_Proc);

void      log_set_proc(Log_Proc *proc);
Log_Proc *log_get_proc(void);

LOG_PROC(log_default_proc);
LOG_PROC(log_empty_proc);

void log_set_minimum_level(Log_Level level);

void log_printf(Log_Level level, const char *fmt, ...) PRINTF_FORMAT(2, 3);

#define log_info(...)    log_printf(LOG_INFO, __VA_ARGS__)
#define log_warning(...) log_printf(LOG_WARNING, __VA_ARGS__)
#define log_error(...)   log_printf(LOG_ERROR, __VA_ARGS__)

C_LINKAGE_END

#if LANG_CPP

// Arrays

#define for_array(it, arr) for (auto *it = (arr).data; it != (arr).data + (arr).len; it++)

template <typename T>
struct Array {
    Allocator allocator;

    T *data;
    isize len;
    isize cap;

    T &operator[](isize index) {
        ASSERT(index < len);
        return data[index];
    }

    const T &operator[](isize index) const {
        ASSERT(index < len);
        return data[index];
    }
};

template <typename T> static bool array_can_grow_in_place(Array<T> *arr);
template <typename T> static void array_grow(Array<T> *arr, isize min_cap);
template <typename T> void array_init(Array<T> *arr, const Allocator &al, isize initial_len = 0, isize initial_cap = 0);
template <typename T> void array_free(Array<T> *arr);
template <typename T> void array_reserve(Array<T> *arr, isize new_cap);
template <typename T> void array_add(Array<T> *arr, const T &value);
template <typename T> T    array_pop(Array<T> *arr);
template <typename T> void array_clear(Array<T> *arr);
template <typename T> void array_ordered_remove(Array<T> *arr, isize index);
template <typename T> void array_unordered_remove(Array<T> *arr, isize index);

// TODO: unused, to be removed
template <typename T>
static bool array_can_grow_in_place(Array<T> *arr) {
    if (!arr->data) return false;

    u8 *end = (u8 *)arr->data + sizeof(T) * arr->cap;
    u8 *arena_top = (u8 *)arr->arena + arr->arena->pos;

    return end == arena_top;
}

template <typename T>
static void array_grow(Array<T> *arr, isize cap_wanted) {
    isize new_cap = arr->cap > 0 ? arr->cap * 2 : 8;
    if (new_cap < cap_wanted) new_cap = cap_wanted;
    array_reserve(arr, new_cap);
}

template <typename T>
void array_init(Array<T> *arr, const Allocator &al, isize initial_len, isize initial_cap) {
    ASSERT(initial_len >= 0 && initial_cap >= 0);

    arr->allocator = al;
    arr->data = NULL;
    arr->len  = 0;
    arr->cap  = 0;

    initial_cap = MAX(initial_len, initial_cap);

    if (initial_cap > 0) {
        isize size = sizeof(T) * initial_cap;
        arr->data = (T *)allocator_alloc(al, size);
        ASSERT(arr->data);
        mem_set(arr->data, 0, size);

        arr->cap = initial_cap;
        arr->len = initial_len;
    }
}

template <typename T>
void array_free(Array<T> *arr) {
    if (arr->data) {
        if (arr->allocator.proc) allocator_free(arr->allocator, arr->data);
    }

    arr->data = NULL;
    arr->len  = 0;
    arr->cap  = 0;
}

template <typename T>
void array_reserve(Array<T> *arr, isize new_cap) {
    if (new_cap <= arr->cap) return;

    isize old_size = arr->cap * sizeof(T);
    isize new_size = new_cap * sizeof(T);

    // Try to resize first and if it failed we fall back by doing alloc + move + free,
    // because there are some cases where allocator just returns NULL when we try
    // to resize, this kind of thing is common in arena allocator.
    void *ptr = allocator_resize(arr->allocator, arr->data, old_size, new_size);
    if (!ptr) {
        if (new_cap > 0) {
            ptr = allocator_alloc(arr->allocator, new_cap * sizeof(T));
            ASSERT(ptr);
            mem_move(ptr, arr->data, old_size);
        }
        allocator_free(arr->allocator, arr->data);
    }

    arr->data = (T *)ptr;
    arr->cap  = new_cap;
}

template <typename T>
void array_add(Array<T> *arr, const T &value) {
    if (arr->len >= arr->cap) {
        array_grow(arr, arr->len + 1);
    }
    arr->data[arr->len] = value;
    arr->len += 1;
}

template <typename T>
T array_pop(Array<T> *arr) {
    ASSERT(arr->len > 0);

    arr->len -= 1;
    return arr->data[arr->len];
}

template <typename T>
void array_clear(Array<T> *arr) {
    arr->len = 0;
}

template <typename T>
void array_ordered_remove(Array<T> *arr, isize index) {
    ASSERT(index < arr->len);

    for (isize i = index; i < arr->len - 1; i++) {
        arr->data[i] = arr->data[i + 1];
    }

    arr->len -= 1;
}

template <typename T>
void array_unordered_remove(Array<T> *arr, isize index) {
    ASSERT(index < arr->len);

    arr->data[index] = arr->data[arr->len - 1];
    arr->len -= 1;
}

// Strings

struct String {
    u8 *data;
    isize len;

    const u8 &operator[](isize index) const {
        ASSERT(0 <= index && index < len);
        return data[index];
    }
};

// Example: String s = LIT("Hello!");
#define LIT(s) String{ (u8 *)(s), sizeof(s)-1 }

// Example: printf("%.*s\n", FMT(s));
#define FMT(s) (int)(s).len, (const char *)(s).data

bool byte_is_lower(u8 c);
bool byte_is_upper(u8 c);
bool byte_is_alpha(u8 c);
bool byte_is_digit(u8 c);
bool byte_is_alnum(u8 c);
bool byte_is_space(u8 c);
bool byte_in_set(u8 c, const String &strset);
u8   byte_to_lower(u8 c);
u8   byte_to_upper(u8 c);

String string_make(u8 *str, isize len);
String string_empty();
String string_from_cstr(const char *cstr);
const char *string_to_cstr(const Allocator &al, const String &s);

i32    string_compare(const String &a, const String &b);
bool   string_contains(const String &s, const String &substr);
bool   string_contains_byte(const String &s, u8 c);
String string_cut_prefix(const String &s, const String &prefix);
String string_cut_suffix(const String &s, const String &suffix);
bool   string_has_prefix(const String &s, const String &prefix);
bool   string_has_suffix(const String &s, const String &suffix);
isize  string_index(const String &s, const String &substr);
isize  string_index_byte(const String &s, u8 c);
isize  string_last_index(const String &s, const String &substr);
isize  string_last_index_byte(const String &s, u8 c);
String string_trim(const String &s, const String &cutset);
String string_trim_left(const String &s, const String &cutset);
String string_trim_right(const String &s, const String &cutset);
String string_trim_space(const String &s);
String string_trim_prefix(const String &s, const String &prefix);
String string_trim_suffix(const String &s, const String &suffix);

String string_to_lower(const Allocator &al, const String &s);
String string_to_upper(const Allocator &al, const String &s);
String string_clone(const Allocator &al, const String &s);
String string_concat(const Allocator &al, const String &a, const String &b);
String string_join(const Allocator &al, const Array<String> &elems, const String &sep);
Array<String> string_split(const Allocator &al, const String &s, const String &sep);
String string_replace(const Allocator &al, const String &s, const String &oldstr, const String &newstr);

inline bool string_eq(const String &a, const String &b) { return a.len == b.len && mem_compare(a.data, b.data, a.len) == 0; }
inline bool string_ne(const String &a, const String &b) { return !string_eq(a,b);          }
inline bool string_lt(const String &a, const String &b) { return string_compare(a,b) <  0; }
inline bool string_gt(const String &a, const String &b) { return string_compare(a,b) >  0; }
inline bool string_le(const String &a, const String &b) { return string_compare(a,b) <= 0; }
inline bool string_ge(const String &a, const String &b) { return string_compare(a,b) >= 0; }

inline bool operator == (const String &a, const String &b) { return string_eq(a,b); }
inline bool operator != (const String &a, const String &b) { return string_ne(a,b); }
inline bool operator <  (const String &a, const String &b) { return string_lt(a,b); }
inline bool operator >  (const String &a, const String &b) { return string_gt(a,b); }
inline bool operator <= (const String &a, const String &b) { return string_le(a,b); }
inline bool operator >= (const String &a, const String &b) { return string_ge(a,b); }

template <isize N> inline bool operator == (const String &a, const char (&b)[N]) { return string_eq(a, string_make((u8 *)b, N-1)); }
template <isize N> inline bool operator != (const String &a, const char (&b)[N]) { return string_ne(a, string_make((u8 *)b, N-1)); }
template <isize N> inline bool operator <  (const String &a, const char (&b)[N]) { return string_lt(a, string_make((u8 *)b, N-1)); }
template <isize N> inline bool operator >  (const String &a, const char (&b)[N]) { return string_gt(a, string_make((u8 *)b, N-1)); }
template <isize N> inline bool operator <= (const String &a, const char (&b)[N]) { return string_le(a, string_make((u8 *)b, N-1)); }
template <isize N> inline bool operator >= (const String &a, const char (&b)[N]) { return string_ge(a, string_make((u8 *)b, N-1)); }
template <>        inline bool operator == (const String &a, const char (&b)[1]) { return a.len == 0; }
template <>        inline bool operator != (const String &a, const char (&b)[1]) { return a.len != 0; }

// Hash tables

enum {
    TABLE_SLOT_EMPTY,
    TABLE_SLOT_OCCUPIED,
    TABLE_SLOT_TOMBSTONE,
};

template <typename K, typename V>
struct Table_Entry {
    K key;
    V value;
    u8 state;
};

template <typename K, typename V>
struct Table {
    Allocator allocator;

    Table_Entry<K, V> *entries;
    isize cap;
    isize len;
};

u32 table_hash(u64 x);
u32 table_hash(const String &s);

template <typename T>             u32    table_hash(T *ptr);
template <typename K, typename V> void   table_init(Table<K, V> *t, const Allocator &a, isize cap = 64);
template <typename K, typename V> void   table_free(Table<K, V> *t);
template <typename K, typename V> isize  table_find_slot(Table<K, V> *t, K key);
template <typename K, typename V> bool   table_set(Table<K, V> *t, K key, const V &value);
template <typename K, typename V> V     *table_get(Table<K, V> *t, K key);
template <typename K, typename V> bool   table_remove(Table<K, V> *t, K key);
template <typename K, typename V> void   table_resize(Table<K, V> *t, const Allocator &a, isize new_cap);
template <typename K, typename V> void   table_clear(Table<K, V> *t);

template <typename T>
inline u32 table_hash(T *ptr) {
    return table_hash((u64)ptr);
}

template <typename K, typename V>
void table_init(Table<K, V> *t, const Allocator &a, isize cap) {
    using Entry = Table_Entry<K, V>;

    t->allocator = a;
    t->cap       = cap;
    t->len       = 0;
    t->entries   = alloc_array(a, Entry, cap);

    for (isize i = 0; i < cap; i++) {
        t->entries[i].state = TABLE_SLOT_EMPTY;
    }
}

template <typename K, typename V>
void table_free(Table<K, V> *t) {
    if (t->entries) {
        if (t->allocator.proc) allocator_free(t->allocator, t->entries);
    }
}

template <typename K, typename V>
isize table_find_slot(Table<K, V> *t, K key) {
    u32 h = table_hash(key);
    isize index = h % t->cap;
    isize first_tombstone = -1;

    for (;;) {
        auto *e = &t->entries[index];
        if (e->state == TABLE_SLOT_EMPTY) {
            return first_tombstone != -1 ? first_tombstone : index;
        }
        if (e->state == TABLE_SLOT_TOMBSTONE) {
            if (first_tombstone == -1) {
                first_tombstone = index;
            }
        }
        if (e->key == key) {
            return index;
        }
        index = (index + 1) % t->cap;
    }
}

template <typename K, typename V>
bool table_set(Table<K, V> *t, K key, const V &value) {
    // Must stay below 0.7-0.8 or performance collapses.
    // Reference: https://github.com/djiangtw/data-structures-in-practice-public/blob/main/manuscript/chapters/chapter07.md
    if (t->len >= t->cap * 0.7) {
        table_resize(t, t->allocator, t->cap * 2);
    }

    isize index = table_find_slot(t, key);
    auto *e = &t->entries[index];

    if (e->state != TABLE_SLOT_OCCUPIED) {
        e->state = TABLE_SLOT_OCCUPIED;
        e->key = key;
        t->len++;
    }
    e->value = value;

    return true;
}

template <typename K, typename V>
V *table_get(Table<K, V> *t, K key) {
    u32 h = table_hash(key);
    isize index = h % t->cap;

    for (;;) {
        auto *e = &t->entries[index];
        if (e->state == TABLE_SLOT_EMPTY) return NULL;
        if (e->state == TABLE_SLOT_OCCUPIED) {
            if (e->key == key) return &e->value;
        }
        index = (index + 1) % t->cap;
    }
}

template <typename K, typename V>
bool table_remove(Table<K, V> *t, K key) {
    u32 h = table_hash(key);
    isize index = h % t->cap;

    for (;;) {
        auto *e = &t->entries[index];
        if (e->state == TABLE_SLOT_EMPTY) return false;
        if (e->state == TABLE_SLOT_OCCUPIED) {
            if (e->key == key) {
                e->state = TABLE_SLOT_TOMBSTONE;
                t->len--;
                return true;
            }
        }
        index = (index + 1) % t->cap;
    }
}

template <typename K, typename V>
void table_resize(Table<K, V> *t, const Allocator &a, isize new_cap) {
    using Entry = Table_Entry<K, V>;

    auto *old_entries = t->entries;
    isize old_cap = t->cap;

    t->entries = alloc_array(a, Entry, new_cap);
    t->cap     = new_cap;
    t->len     = 0;

    for (isize i = 0; i < new_cap; i++) {
        t->entries[i].state = TABLE_SLOT_EMPTY;
    }

    for (isize i = 0; i < old_cap; i++) {
        auto *e = &old_entries[i];
        if (e->state == TABLE_SLOT_OCCUPIED) {
            table_set(t, e->key, e->value);
        }
    }

    allocator_free(a, old_entries);
}

template <typename K, typename V>
void table_clear(Table<K, V> *t) {
    for (isize i = 0; i < t->cap; i++) {
        t->entries[i].state = TABLE_SLOT_EMPTY;
    }

    t->len = 0;
}

#endif

#endif // BASE_H
