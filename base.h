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
//
// IMPORTANT
//
// Do this:
//
//   #define BASE_IMPLEMENTATION
//
// in ONE C/C++ file before including this library.
//
// It should look like this:
//
//   #define BASE_IMPLEMENTATION
//   #include "base.h"
//
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

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

//
// DECLARATION
//

#ifdef BASE_IMPLEMENTATION
    #define BASE_DEF
#else
    #define BASE_DEF extern
#endif

#define local_persist static // Local persisting variable
#define internal      static // Internal linkage
#define global        static // Global variable

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

#if defined(__cplusplus)
    #define LANG_C    0
    #define C_VERSION 0

    #define LANG_CPP 1
    #if __cplusplus >= 202302L
        #define CPP_VERSION 23
    #elif __cplusplus >= 202002L
        #define CPP_VERSION 20
    #elif __cplusplus >= 201703L
        #define CPP_VERSION 17
    #elif __cplusplus >= 201402L
        #define CPP_VERSION 14
    #elif __cplusplus >= 201103L
        #define CPP_VERSION 11
    #else
        #define CPP_VERSION 98
    #endif
#else
    #define LANG_CPP    0
    #define CPP_VERSION 0

    #define LANG_C 1
    #if defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 202311L
            #define C_VERSION 23
        #elif __STDC_VERSION__ >= 201710L
            #define C_VERSION 17
        #elif __STDC_VERSION__ >= 201112L
            #define C_VERSION 11
        #elif __STDC_VERSION__ >= 199901L
            #define C_VERSION 99
        #else
            #define C_VERSION 90
        #endif
    #else
        #define C_VERSION 90
    #endif
#endif

#if LANG_C && C_VERSION < 11
    #pragma message("Warning: C11 or later recommended")
#elif LANG_CPP && CPP_VERSION < 11
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

#if CPP_VERSION >= 11
    #define THREAD_LOCAL thread_local
#elif C_VERSION >= 11
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
                assert_proc("Assertion failed", #expr, __FILE__, (int)__LINE__, fmt, ##__VA_ARGS__);\
                TRAP();\
            }\
        } while (0)
    #define PANIC(fmt, ...)\
        do {\
            assert_proc("Panic", NULL, __FILE__, (int)__LINE__, fmt, ##__VA_ARGS__);\
            TRAP();\
        } while (0)
#else
    #define ASSERT(expr)               (expr)
    #define ASSERT_MSG(expr, fmt, ...) (expr)
    #define PANIC(fmt, ...)
#endif

// The implementation is declared here because ASSERT() is used
// inside Array and String declaration.
internal inline void assert_proc(const char *prefix, const char *expr, const char *file, int line, const char *fmt, ...) {
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

#if CPP_VERSION >= 11
    #define STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#elif C_VERSION >= 11
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
#define F64_MIN     (2.2250738585072014e-308)
#define F64_MAX     (1.7976931348623157e+308)
#define F64_EPSILON (2.2204460492503131e-16)

C_LINKAGE_BEGIN

// Memory

#define DEFAULT_MEMORY_ALIGNMENT (2 * sizeof(void *))

typedef void *Raw_Alloc_Proc (size_t sz);
typedef void *Raw_Resize_Proc(void *ptr, size_t newsz);
typedef void  Raw_Free_Proc  (void *ptr);

BASE_DEF void mem_set_procs(Raw_Alloc_Proc *alloc_proc, Raw_Resize_Proc *resize_proc, Raw_Free_Proc *free_proc);

BASE_DEF void *mem_alloc(isize sz);
BASE_DEF void *mem_resize(void *ptr, isize newsz);
BASE_DEF void  mem_free(void *ptr);

#define mem_copy    memcpy
#define mem_move    memmove
#define mem_set     memset
#define mem_compare memcmp

// TODO: unused, to be removed
#if 0
BASE_DEF isize mem_page_size(void);
BASE_DEF isize mem_granularity(void);

BASE_DEF void *vmem_reserve(isize size);
BASE_DEF bool  vmem_commit(void *ptr, isize size);
BASE_DEF bool  vmem_decommit(void *ptr, isize size);
BASE_DEF bool  vmem_release(void *ptr);
#endif

// Arena

#define ARENA_BASE_POS ALIGN_UP(sizeof(Arena), ARENA_ALIGN)
#define ARENA_ALIGN    (sizeof(void *))

typedef struct {
    isize reserve_size;
    isize commit_size;
    isize pos;
    isize commit_pos;
} Arena;

BASE_DEF Arena *arena_create(isize reserve_size, isize commit_size);
BASE_DEF void   arena_destroy(Arena *a);
BASE_DEF void  *arena_push(Arena *a, isize size, bool non_zero = false);
BASE_DEF void   arena_pop(Arena *a, isize size);
BASE_DEF void   arena_pop_to(Arena *a, isize pos);
BASE_DEF void   arena_clear(Arena *a);

#define arena_push_type(a, T)        (T *)arena_push((a), sizeof(T), false)
#define arena_push_type_nz(a, T)     (T *)arena_push((a), sizeof(T), true)
#define arena_push_array(a, T, n)    (T *)arena_push((a), sizeof(T) * (n), false)
#define arena_push_array_nz(a, T, n) (T *)arena_push((a), sizeof(T) * (n), true)

typedef struct {
    Arena *arena;
    isize start_pos;
} Arena_Temp;

BASE_DEF Arena_Temp arena_begin_temp(Arena *a);
BASE_DEF void       arena_end_temp(Arena_Temp temp);

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

BASE_DEF Arena_Temp arena_begin_scratch(Arena **conflicts, i32 num_conflicts);
BASE_DEF void       arena_end_scratch(Arena_Temp scratch);

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

BASE_DEF void *allocator_alloc(Allocator a, isize sz);
BASE_DEF void *allocator_resize(Allocator a, void *ptr, isize oldsz, isize newsz);
BASE_DEF void  allocator_free(Allocator a, void *ptr);
BASE_DEF void  allocator_free_all(Allocator a);

#define alloc_type(a, T)     (T *)allocator_alloc((a), sizeof(T))
#define alloc_array(a, T, n) (T *)allocator_alloc((a), (n) * sizeof(T))

BASE_DEF Allocator heap_allocator(void);
BASE_DEF ALLOCATOR_PROC(heap_allocator_proc);

#define heap_alloc(sz)                 allocator_alloc(heap_allocator(), (sz))
#define heap_resize(ptr, oldsz, newsz) allocator_resize(heap_allocator(), (ptr), (oldsz), (newsz))
#define heap_free(ptr)                 allocator_free(heap_allocator(), (ptr))

BASE_DEF Allocator arena_allocator(Arena *arena);
BASE_DEF ALLOCATOR_PROC(arena_allocator_proc);

// Logs

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
} Log_Level;

#define LOG_PROC(name)\
    void name(Log_Level level, const char *fmt, va_list args)

typedef LOG_PROC(Log_Proc);

BASE_DEF void      log_set_proc(Log_Proc *proc);
BASE_DEF Log_Proc *log_get_proc(void);

BASE_DEF LOG_PROC(log_default_proc);
BASE_DEF LOG_PROC(log_empty_proc);

BASE_DEF void log_set_min_level(Log_Level level);

BASE_DEF void log_printf(Log_Level level, const char *fmt, ...) PRINTF_FORMAT(2, 3);

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

template <typename T>
internal void array_grow(Array<T> *arr, isize min_cap);

template <typename T>
internal bool array_can_grow_in_place(Array<T> *arr);

template <typename T>
void array_init(Array<T> *arr, const Allocator &al, isize initial_len = 0, isize initial_cap = 0);

template <typename T>
void array_free(Array<T> *arr);

template <typename T>
void array_reserve(Array<T> *arr, isize new_cap);

template <typename T>
void array_add(Array<T> *arr, const T &value);

template <typename T>
T array_pop(Array<T> *arr);

template <typename T>
void array_clear(Array<T> *arr);

template <typename T>
void array_ordered_remove(Array<T> *arr, isize index);

template <typename T>
void array_unordered_remove(Array<T> *arr, isize index);

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

inline bool byte_is_lower(u8 c);
inline bool byte_is_upper(u8 c);
inline bool byte_is_alpha(u8 c);
inline bool byte_is_digit(u8 c);
inline bool byte_is_alnum(u8 c);
inline bool byte_is_space(u8 c);
inline bool byte_in_set(u8 c, const String &strset);
inline u8   byte_to_lower(u8 c);
inline u8   byte_to_upper(u8 c);

inline bool string_eq(const String &a, const String &b);
inline bool string_ne(const String &a, const String &b);
inline bool string_lt(const String &a, const String &b);
inline bool string_gt(const String &a, const String &b);
inline bool string_le(const String &a, const String &b);
inline bool string_ge(const String &a, const String &b);

BASE_DEF String string_make(u8 *str, isize len);
BASE_DEF String string_empty();
BASE_DEF String string_from_cstr(const char *cstr);
BASE_DEF const char *string_to_cstr(const Allocator &al, const String &s);

// Non-allocating
BASE_DEF i32    string_compare(const String &a, const String &b);
BASE_DEF bool   string_contains(const String &s, const String &substr);
BASE_DEF bool   string_contains_byte(const String &s, u8 c);
BASE_DEF String string_cut_prefix(const String &s, const String &prefix);
BASE_DEF String string_cut_suffix(const String &s, const String &suffix);
BASE_DEF bool   string_has_prefix(const String &s, const String &prefix);
BASE_DEF bool   string_has_suffix(const String &s, const String &suffix);
BASE_DEF isize  string_index(const String &s, const String &substr);
BASE_DEF isize  string_index_byte(const String &s, u8 c);
BASE_DEF isize  string_last_index(const String &s, const String &substr);
BASE_DEF isize  string_last_index_byte(const String &s, u8 c);
BASE_DEF String string_trim(const String &s, const String &cutset);
BASE_DEF String string_trim_left(const String &s, const String &cutset);
BASE_DEF String string_trim_right(const String &s, const String &cutset);
BASE_DEF String string_trim_space(const String &s);
BASE_DEF String string_trim_prefix(const String &s, const String &prefix);
BASE_DEF String string_trim_suffix(const String &s, const String &suffix);

// Allocation-based
BASE_DEF String string_to_lower(const Allocator &al, const String &s);
BASE_DEF String string_to_upper(const Allocator &al, const String &s);
BASE_DEF String string_clone(const Allocator &al, const String &s);
BASE_DEF String string_concat(const Allocator &al, const String &a, const String &b);
BASE_DEF String string_join(const Allocator &al, const Array<String> &elems, const String &sep);
BASE_DEF Array<String> string_split(const Allocator &al, const String &s, const String &sep);
BASE_DEF String string_replace(const Allocator &al, const String &s, const String &oldstr, const String &newstr);

inline bool operator == (const String &a, const String &b);
inline bool operator != (const String &a, const String &b);
inline bool operator <  (const String &a, const String &b);
inline bool operator >  (const String &a, const String &b);
inline bool operator <= (const String &a, const String &b);
inline bool operator >= (const String &a, const String &b);

template <isize N> inline bool operator == (const String &a, const char (&b)[N]);
template <isize N> inline bool operator != (const String &a, const char (&b)[N]);
template <isize N> inline bool operator <  (const String &a, const char (&b)[N]);
template <isize N> inline bool operator >  (const String &a, const char (&b)[N]);
template <isize N> inline bool operator <= (const String &a, const char (&b)[N]);
template <isize N> inline bool operator >= (const String &a, const char (&b)[N]);

template <> inline bool operator == (const String &a, const char (&b)[1]);
template <> inline bool operator != (const String &a, const char (&b)[1]);

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

inline u32 table_hash(u64 x);

template <typename T>
inline u32 table_hash(T *ptr);

inline u32 table_hash(const String &s);

template <typename K, typename V>
void table_init(Table<K, V> *t, const Allocator &a, isize cap = 64);

template <typename K, typename V>
void table_free(Table<K, V> *t);

template <typename K, typename V>
isize table_find_slot(Table<K, V> *t, K key);

template <typename K, typename V>
bool table_set(Table<K, V> *t, K key, const V &value);

template <typename K, typename V>
V *table_get(Table<K, V> *t, K key);

template <typename K, typename V>
bool table_remove(Table<K, V> *t, K key);

template <typename K, typename V>
void table_resize(Table<K, V> *t, const Allocator &a, isize new_cap);

template <typename K, typename V>
void table_clear(Table<K, V> *t);

#endif // LANG_CPP

//
// IMPLEMENTATION
//

#ifdef BASE_IMPLEMENTATION

#include <stdlib.h>

#if OS_WINDOWS
#include <windows.h>
#endif

C_LINKAGE_BEGIN

// Memory

global Raw_Alloc_Proc  *mem_alloc_proc  = &malloc;
global Raw_Resize_Proc *mem_resize_proc = &realloc;
global Raw_Free_Proc   *mem_free_proc   = &free;

BASE_DEF void mem_set_procs(Raw_Alloc_Proc *alloc_proc, Raw_Resize_Proc *resize_proc, Raw_Free_Proc *free_proc) {
    mem_alloc_proc  = alloc_proc;
    mem_resize_proc = resize_proc;
    mem_free_proc   = free_proc;
}

BASE_DEF void *mem_alloc(isize sz) {
    return mem_alloc_proc(sz);
}

BASE_DEF void *mem_resize(void *ptr, isize newsz) {
    return mem_resize_proc(ptr, newsz);
}

BASE_DEF void mem_free(void *ptr) {
    mem_free_proc(ptr);
}

#if OS_WINDOWS

// TODO: unused, to be removed
#if 0
BASE_DEF isize mem_page_size(void) {
    local_persist isize result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwPageSize;
    }
    return result;
}

BASE_DEF isize mem_granularity(void) {
    local_persist isize result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwAllocationGranularity;
    }
    return result;
}

BASE_DEF void *vmem_reserve(isize size) {
    size = ALIGN_UP(size, mem_page_size());
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

BASE_DEF bool vmem_commit(void *ptr, isize size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    void *ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(ret == ptr);
    return ret != NULL;
}

BASE_DEF bool vmem_decommit(void *ptr, isize size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    return VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

BASE_DEF bool vmem_release(void *ptr) {
    ASSERT(ptr);
    return VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}
#endif // 0

#endif // OS_WINDOWS

// Arena

BASE_DEF Arena *arena_create(isize reserve_size, isize commit_size) {
// TODO: unused, to be removed
#if 0
    isize page_size = mem_page_size();
    isize gran      = mem_granularity();

    reserve_size = ALIGN_UP(reserve_size, gran);
    commit_size  = ALIGN_UP(commit_size, page_size);

    Arena *a = (Arena *)vmem_reserve(reserve_size);
    ASSERT(vmem_commit(a, commit_size));
#else
    reserve_size = ALIGN_UP(reserve_size, ARENA_ALIGN);
    commit_size  = ALIGN_UP(commit_size, ARENA_ALIGN);

    Arena *a = (Arena *)mem_alloc(reserve_size);
    ASSERT(a);
#endif

    a->reserve_size = reserve_size;
    a->commit_size  = commit_size;
    a->pos          = ARENA_BASE_POS;
    a->commit_pos   = commit_size;

    return a;
}

BASE_DEF void arena_destroy(Arena *a) {
// TODO: unused, to be removed
#if 0
    vmem_release(a);
#else
    mem_free(a);
#endif
}

BASE_DEF void *arena_push(Arena *a, isize size, bool non_zero) {
    isize pos_aligned = ALIGN_UP(a->pos, ARENA_ALIGN);
    isize new_pos = pos_aligned + size;

    if (new_pos > a->reserve_size) return NULL;

    if (new_pos > a->commit_pos) {
        isize new_pos_aligned = ALIGN_UP(new_pos, a->commit_size);
        isize new_commit_pos = MIN(new_pos_aligned, a->reserve_size);

// TODO: unused, to be removed
#if 0
        u8 *mem = (u8 *)a + a->commit_pos;
        isize commit_size = new_commit_pos - a->commit_pos;
        ASSERT(vmem_commit(mem, commit_size));
#endif

        a->commit_pos = new_commit_pos;
    }

    a->pos = new_pos;

    u8 *out = (u8 *)a + pos_aligned;
    if (!non_zero) mem_set(out, 0, size);
    return out;
}

BASE_DEF void arena_pop(Arena *a, isize size) {
    size = MIN(size, a->pos - ARENA_BASE_POS);
    a->pos -= size;
}

BASE_DEF void arena_pop_to(Arena *a, isize pos) {
    isize size = pos < a->pos ? a->pos - pos : 0;
    arena_pop(a, size);
}

BASE_DEF void arena_clear(Arena *a) {
    arena_pop_to(a, ARENA_BASE_POS);
}

BASE_DEF Arena_Temp arena_begin_temp(Arena *a) {
    Arena_Temp temp;
    temp.arena = a;
    temp.start_pos = a->pos;
    return temp;
}

BASE_DEF void arena_end_temp(Arena_Temp temp) {
    arena_pop_to(temp.arena, temp.start_pos);
}

THREAD_LOCAL Arena *arena_scratch_pool[ARENA_SCRATCH_POOL] = { NULL, NULL };

BASE_DEF Arena_Temp arena_begin_scratch(Arena **conflicts, i32 num_conflicts) {
    isize scratch_index = -1;

    for (isize i = 0; i < ARENA_SCRATCH_POOL; i++) {
        bool found = false;

        for (i32 j = 0; j < num_conflicts; j++) {
            if (arena_scratch_pool[i] == conflicts[j]) {
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
        PANIC("No available scratch arena");
        Arena_Temp t = {0};
        return t;
    }

    Arena **selected = &arena_scratch_pool[scratch_index];
    if (*selected == NULL) {
        *selected = arena_create(ARENA_SCRATCH_RESERVE_SIZE, ARENA_SCRATCH_COMMIT_SIZE);
        ASSERT(*selected);
    }
    return arena_begin_temp(*selected);
}

BASE_DEF void arena_end_scratch(Arena_Temp scratch) {
    arena_end_temp(scratch);
}

// Custom allocation

BASE_DEF void *allocator_alloc(Allocator a, isize sz) {
    return a.proc(a.data, ALLOCATION_ALLOC, sz, DEFAULT_MEMORY_ALIGNMENT, NULL, 0);
}

BASE_DEF void *allocator_resize(Allocator a, void *ptr, isize oldsz, isize newsz) {
    return a.proc(a.data, ALLOCATION_RESIZE, newsz, DEFAULT_MEMORY_ALIGNMENT, ptr, oldsz);
}

BASE_DEF void allocator_free(Allocator a, void *ptr) {
    if (ptr) {
        a.proc(a.data, ALLOCATION_FREE, 0, 0, ptr, 0);
    }
}

BASE_DEF void allocator_free_all(Allocator a) {
    a.proc(a.data, ALLOCATION_FREE_ALL, 0, 0, NULL, 0);
}

BASE_DEF Allocator heap_allocator(void) {
    Allocator a;
    a.proc = heap_allocator_proc;
    a.data = NULL;
    return a;
}

BASE_DEF ALLOCATOR_PROC(heap_allocator_proc) {
    UNUSED(alignment);
    UNUSED(oldsz);

    void *ptr = NULL;

    switch (alloc_mode) {
        case ALLOCATION_ALLOC:
            ptr = mem_alloc(newsz);
            break;
        case ALLOCATION_RESIZE:
            ptr = mem_resize(oldmem, newsz);
            break;
        case ALLOCATION_FREE:
            mem_free(oldmem);
            break;
        case ALLOCATION_FREE_ALL:
            // nothing
            break;
    }

    return ptr;
}

BASE_DEF Allocator arena_allocator(Arena *arena) {
    Allocator a;
    a.proc = arena_allocator_proc;
    a.data = arena;
    return a;
}

BASE_DEF ALLOCATOR_PROC(arena_allocator_proc) {
    UNUSED(alignment);
    UNUSED(oldmem);
    UNUSED(oldsz);

    Arena *a = (Arena *)alloc_data;
    void *ptr = NULL;

    switch (alloc_mode) {
        case ALLOCATION_ALLOC:
            ptr = arena_push(a, newsz);
            break;
        case ALLOCATION_RESIZE: {
        } break;

        case ALLOCATION_FREE:
            // nothing
            break;
        case ALLOCATION_FREE_ALL:
            arena_clear(a);
            break;
    }

    return ptr;
}

// Logs

global Log_Proc *log_proc = &log_default_proc;

BASE_DEF void log_set_proc(Log_Proc *proc) {
    log_proc = proc;
}

BASE_DEF Log_Proc *log_get_proc(void) {
    return log_proc;
}

global Log_Level log_min_level = LOG_INFO;

BASE_DEF void log_set_min_level(Log_Level level) {
    log_min_level = level;
}

BASE_DEF LOG_PROC(log_default_proc) {
    switch (level) {
        case LOG_INFO:
            fprintf(stderr, "[INFO] ");
            break;
        case LOG_WARNING:
            fprintf(stderr, "[WARNING] ");
            break;
        case LOG_ERROR:
            fprintf(stderr, "[ERROR] ");
            break;
        default:
            PANIC("Invalid log level");
            return;
    }

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

BASE_DEF LOG_PROC(log_empty_proc) {
    UNUSED(level);
    UNUSED(fmt);
    UNUSED(args);
}

BASE_DEF void log_printf(Log_Level level, const char *fmt, ...) {
    if (level < log_min_level) return;

    va_list args;
    va_start(args, fmt);
    log_proc(level, fmt, args);
    va_end(args);
}

C_LINKAGE_END

#endif // BASE_IMPLEMENTATION

#if LANG_CPP

// Arrays

// TODO: unused, to be removed
template <typename T>
internal bool array_can_grow_in_place(Array<T> *arr) {
    if (!arr->data) return false;

    u8 *end = (u8 *)arr->data + sizeof(T) * arr->cap;
    u8 *arena_top = (u8 *)arr->arena + arr->arena->pos;

    return end == arena_top;
}

template <typename T>
internal void array_grow(Array<T> *arr, isize cap_wanted) {
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

inline bool byte_is_lower(u8 c) {
    return 'a' <= c && c <= 'z';
}

inline bool byte_is_upper(u8 c) {
    return 'A' <= c && c <= 'Z';
}

inline bool byte_is_alpha(u8 c) {
    return byte_is_lower(c) || byte_is_upper(c);
}

inline bool byte_is_digit(u8 c) {
    return '0' <= c && c <= '9';
}

inline bool byte_is_alnum(u8 c) {
    return byte_is_alpha(c) || byte_is_digit(c);
}

inline bool byte_is_space(u8 c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool byte_in_set(u8 c, const String &strset) {
    for (isize i = 0; i < strset.len; i++) {
        if (strset.data[i] == c) return true;
    }
    return false;
}

inline u8 byte_to_lower(u8 c) {
    return byte_is_upper(c) ? c + 32 : c;
}

inline u8 byte_to_upper(u8 c) {
    return byte_is_lower(c) ? c - 32 : c;
}

inline bool string_eq(const String &a, const String &b) { return a.len == b.len && mem_compare(a.data, b.data, a.len) == 0; }
inline bool string_ne(const String &a, const String &b) { return !string_eq(a,b);          }
inline bool string_lt(const String &a, const String &b) { return string_compare(a,b) <  0; }
inline bool string_gt(const String &a, const String &b) { return string_compare(a,b) >  0; }
inline bool string_le(const String &a, const String &b) { return string_compare(a,b) <= 0; }
inline bool string_ge(const String &a, const String &b) { return string_compare(a,b) >= 0; }

#ifdef BASE_IMPLEMENTATION

BASE_DEF String string_make(u8 *str, isize len) {
    ASSERT(len >= 0);
    return String{ str, len };
}

BASE_DEF String string_empty() {
    return string_make((u8 *)NULL, 0);
}

BASE_DEF String string_from_cstr(const char *cstr) {
    return string_make((u8 *)cstr, (isize)strlen(cstr));
}

BASE_DEF const char *string_to_cstr(const Allocator &al, const String &s) {
    char *buf = (char *)allocator_alloc(al, s.len + 1);
    mem_copy(buf, s.data, s.len);
    buf[s.len] = 0;
    return buf;
}

BASE_DEF i32 string_compare(const String &a, const String &b) {
    isize n = MIN(a.len, b.len);

    i32 cmp = mem_compare(a.data, b.data, n);
    if (cmp != 0) return cmp;

    if (a.len < b.len) return -1;
    if (a.len > b.len) return 1;
    return 0;
}

BASE_DEF bool string_contains(const String &s, const String &sub) {
    return string_index(s, sub) >= 0;
}

BASE_DEF bool string_contains_byte(const String &s, u8 c) {
    return string_index_byte(s, c) >= 0;
}

BASE_DEF String string_cut_prefix(const String &s, const String &prefix) {
    if (string_has_prefix(s, prefix)) {
        return string_make(s.data + prefix.len, s.len - prefix.len);
    }
    return s;
}

BASE_DEF String string_cut_suffix(const String &s, const String &suffix) {
    if (string_has_suffix(s, suffix)) {
        return string_make(s.data, s.len - suffix.len);
    }
    return s;
}

BASE_DEF bool string_has_prefix(const String &s, const String &prefix) {
    if (prefix.len > s.len) return false;
    return mem_compare(s.data, prefix.data, prefix.len) == 0;
}

BASE_DEF bool string_has_suffix(const String &s, const String &suffix) {
    if (suffix.len > s.len) return false;
    return mem_compare(s.data + (s.len - suffix.len), suffix.data, suffix.len) == 0;
}

BASE_DEF isize string_index(const String &s, const String &sub) {
    if (sub.len == 0) return 0;
    if (sub.len > s.len) return -1;

    for (isize i = 0; i <= s.len - sub.len; i++) {
        if (mem_compare(s.data + i, sub.data, sub.len) == 0) {
            return i;
        }
    }
    return -1;
}

BASE_DEF isize string_index_byte(const String &s, u8 c) {
    for (isize i = 0; i < s.len; i++) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

BASE_DEF isize string_last_index(const String &s, const String &sub) {
    if (sub.len == 0) return s.len;
    if (sub.len > s.len) return -1;

    for (isize i = s.len - sub.len; i >= 0; i--) {
        if (mem_compare(s.data + i, sub.data, sub.len) == 0) {
            return i;
        }
    }
    return -1;
}

BASE_DEF isize string_last_index_byte(const String &s, u8 c) {
    for (isize i = s.len - 1; i >= 0; i--) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

BASE_DEF String string_trim_left(const String &s, const String &cutset) {
    isize i = 0;
    while (i < s.len && byte_in_set(s.data[i], cutset)) i++;
    return string_make(s.data + i, s.len - i);
}

BASE_DEF String string_trim_right(const String &s, const String &cutset) {
    isize end = s.len;
    while (end > 0 && byte_in_set(s.data[end - 1], cutset)) end--;
    return string_make(s.data, end);
}

BASE_DEF String string_trim_space(const String &s) {
    isize start = 0;
    while (start < s.len && byte_is_space(s.data[start])) start++;

    isize end = s.len;
    while (end > start && byte_is_space(s.data[end - 1])) end--;

    return string_make(s.data + start, end - start);
}

BASE_DEF String string_trim(const String &s, const String &cutset) {
    return string_trim_right(string_trim_left(s, cutset), cutset);
}

BASE_DEF String string_trim_prefix(const String &s, const String &prefix) {
    if (string_has_prefix(s, prefix)) {
        return string_make(s.data + prefix.len, s.len - prefix.len);
    }
    return s;
}

BASE_DEF String string_trim_suffix(const String &s, const String &suffix) {
    if (string_has_suffix(s, suffix)) {
        return string_make(s.data, s.len - suffix.len);
    }
    return s;
}

BASE_DEF String string_to_lower(const Allocator &al, const String &s) {
    u8 *data = (u8 *)allocator_alloc(al, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = byte_to_lower(s.data[i]);
    return string_make(data, s.len);
}

BASE_DEF String string_to_upper(const Allocator &al, const String &s) {
    u8 *data = (u8 *)allocator_alloc(al, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = byte_to_upper(s.data[i]);
    return string_make(data, s.len);
}

BASE_DEF String string_clone(const Allocator &al, const String &s) {
    u8 *data = (u8 *)allocator_alloc(al, s.len);
    mem_copy(data, s.data, s.len);
    return string_make(data, s.len);
}

BASE_DEF String string_concat(const Allocator &al, const String &a, const String &b) {
    isize len = a.len + b.len;
    u8 *data = (u8 *)allocator_alloc(al, len);

    mem_copy(data, a.data, a.len);
    mem_copy(data + a.len, b.data, b.len);

    return string_make(data, len);
}

BASE_DEF String string_join(const Allocator &al, const Array<String> &elems, const String &sep) {
    if (elems.len == 0) return string_empty();

    // compute total length
    isize total = 0;
    for (isize i = 0; i < elems.len; i++) {
        total += elems[i].len;
    }
    total += sep.len * (elems.len - 1);

    u8 *data = (u8 *)allocator_alloc(al, total);

    isize pos = 0;

    for (isize i = 0; i < elems.len; i++) {
        // copy element
        mem_copy(data + pos, elems[i].data, elems[i].len);
        pos += elems[i].len;

        // copy separator
        if (i != elems.len - 1) {
            mem_copy(data + pos, sep.data, sep.len);
            pos += sep.len;
        }
    }

    return string_make(data, total);
}

BASE_DEF Array<String> string_split(const Allocator &al, const String &s, const String &sep) {
    Array<String> result;
    array_init(&result, al);

    if (sep.len == 0) {
        // split into bytes
        for (isize i = 0; i < s.len; i++) {
            array_add(&result, string_make(s.data + i, 1));
        }
        return result;
    }

    isize start = 0;

    while (start <= s.len) {
        isize idx = string_index(
            string_make(s.data + start, s.len - start), sep
        );

        if (idx < 0) {
            // last segment
            array_add(&result, string_make(
                        s.data + start,
                        s.len - start));
            break;
        }

        array_add(&result, string_make(
                    s.data + start,
                    idx));

        start += idx + sep.len;
    }

    return result;
}

BASE_DEF String string_replace(const Allocator &al,
                               const String &s,
                               const String &oldstr,
                               const String &newstr) {
    if (oldstr.len == 0) return s;

    // count occurrences
    isize count = 0;
    isize pos = 0;

    while (pos <= s.len - oldstr.len) {
        if (mem_compare(s.data + pos, oldstr.data, oldstr.len) == 0) {
            count++;
            pos += oldstr.len;
        } else {
            pos++;
        }
    }

    if (count == 0) return s;

    isize new_len =
        s.len +
        count * (newstr.len - oldstr.len);

    u8 *data = (u8 *)allocator_alloc(al, new_len);

    isize src = 0;
    isize dst = 0;

    while (src < s.len) {
        if (src <= s.len - oldstr.len &&
            mem_compare(s.data + src, oldstr.data, oldstr.len) == 0) {

            mem_copy(data + dst, newstr.data, newstr.len);
            dst += newstr.len;
            src += oldstr.len;

        } else {
            data[dst++] = s.data[src++];
        }
    }

    return string_make(data, new_len);
}

#endif // BASE_IMPLEMENTATION

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

template <> inline bool operator == (const String &a, const char (&b)[1]) { return a.len == 0; }
template <> inline bool operator != (const String &a, const char (&b)[1]) { return a.len != 0; }

// Hash tables

// Murmur
inline u32 table_hash(u64 x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return (u32)x;
}

template <typename T>
inline u32 table_hash(T *ptr) {
    return table_hash((u64)ptr);
}

// FNV-1a
inline u32 table_hash(const String &s) {
    u32 h = 2166136261u;
    for (isize i = 0; i < s.len; i++) {
        h ^= (u8)s.data[i];
        h *= 16777619u;
    }
    return h;
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
    //
    // Open addressing: Must stay below 0.7-0.8 or performance collapses
    // Reference: https://github.com/djiangtw/data-structures-in-practice-public/blob/main/manuscript/chapters/chapter07.md
    //
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

#endif // LANG_CPP

#endif // BASE_H
