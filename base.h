//
// base.h
//
// This is a public domain C++ library.
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
// in ONE C++ file before including this library.
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
//       void *ptr = PUSH_ONE(arena, T);
//
//     Push n sizeof(T) bytes onto the arena:
//       void *ptr = PUSH_MANY(arena, T, n);
//
//     Clear the arena:
//       arena_clear(arena);
//
//     Destroy the arena:
//       arena_destroy(arena);
//
//     Per-thread scratch arena for temporary allocation:
//       Temp_Arena temp = acquire_scratch_arena();
//       PUSH_MANY(temp.arena, u8, 100);
//       release_scratch_arena(temp);
//
//   Arrays
//
//     Init an array of integers:
//       Array<int> nums;
//       array_init(&nums);
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
//     Concatenate two strings (needs arena for allocation):
//       String new_text = string_concat(arena, text, LIT("Bar"));
//
//     Convert to C string (needs arena for allocation):
//       const char *cstr = string_to_cstr(arena, text);
//
//   Hash Tables
//
//     Init a hash table of type Table<String, int>:
//       Table<String, int> height;
//       table_init(&height);
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
#define global_var    static // Global variable

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

// Compiler hints

#if COMPILER_MSVC
    #define FORCE_INLINE    __forceinline
    #define NO_INLINE       __declspec(noinline)
    #define LIKELY(x)       (x)
    #define UNLIKELY(x)     (x)
#elif COMPILER_CLANG || COMPILER_GCC
    #define FORCE_INLINE    __attribute__((always_inline)) inline
    #define NO_INLINE       __attribute__((noinline))
    #define LIKELY(x)       __builtin_expect(!!(x), 1)
    #define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
    #define FORCE_INLINE    inline
    #define NO_INLINE
    #define LIKELY(x)       (x)
    #define UNLIKELY(x)     (x)
#endif

#define UNUSED(x)       ((void)(x))
#define FALLTHROUGH     [[fallthrough]]
#define DEPRECATED(msg) [[deprecated(msg)]]

// Example: struct PACKED MyStruct { ... };
#if COMPILER_MSVC
    #define PACKED __pragma(pack(push, 1)) /* struct */ __pragma(pack(pop))
#elif COMPILER_CLANG || COMPILER_GCC
    #define PACKED __attribute__((packed))
#endif

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

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef size_t usize;
typedef ptrdiff_t isize;

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

// Bit manipulation

#define BIT(n)            (1ULL << (n))
#define BIT_SET(x, n)     ((x) |=  BIT(n))
#define BIT_CLEAR(x, n)   ((x) &= ~BIT(n))
#define BIT_TOGGLE(x, n)  ((x) ^=  BIT(n))
#define BIT_CHECK(x, n)   (((x) >> (n)) & 1)

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

// Packs 4 chars into a u32, useful for file format magic numbers/tags.
// Example: u32 png_tag = FOURCC('P','N','G',' ');
#define FOURCC(a, b, c, d)\
    ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

// Math helpers

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

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

// Asserts

#if COMPILER_MSVC
    #define DEBUG_BREAK() __debugbreak()
#else
    #define DEBUG_BREAK() __builtin_trap()
#endif

#ifndef _DEBUG
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

// Stringify / concat

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x)       STRINGIFY_INNER(x)

#define CONCAT_INNER(a, b) a##b
#define CONCAT(a, b)       CONCAT_INNER(a, b)

// Scope-based defer
// From: https://github.com/gingerBill/gb/blob/master/gb.h

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

// Memory

#define SWAP(T, a, b) do { T _swap_tmp_ = (a); (a) = (b); (b) = _swap_tmp_; } while(0)

inline void *mem_alloc(isize size);
inline void *mem_realloc(void *ptr, isize new_size);
inline void *mem_copy(void *dest, const void *src, isize num_bytes);
inline void mem_free(void *ptr);
inline void mem_set(void *dest, i32 ch, isize count);
inline i32  mem_compare(const void *a, const void *b, isize n);

BASE_DEF isize mem_page_size();
BASE_DEF isize mem_granularity();

BASE_DEF void *mem_reserve(isize size);
BASE_DEF bool mem_commit(void *ptr, isize size);
BASE_DEF bool mem_decommit(void *ptr, isize size);
BASE_DEF bool mem_release(void *ptr);

// Arena

#define ARENA_BASE_POS ALIGN_UP(sizeof(Arena), ARENA_ALIGN)
#define ARENA_ALIGN (sizeof(void *))

struct Arena {
    isize reserve_size;
    isize commit_size;
    isize pos;
    isize commit_pos;
};

BASE_DEF Arena *arena_create(isize reserve_size, isize commit_size);
BASE_DEF void arena_destroy(Arena *a);
BASE_DEF void *arena_push(Arena *a, isize size, bool non_zero = false);
BASE_DEF void arena_pop(Arena *a, isize size);
BASE_DEF void arena_pop_to(Arena *a, isize pos);
BASE_DEF void arena_clear(Arena *a);

#define PUSH_ONE(a, T) (T *)arena_push((a), sizeof(T), false)
#define PUSH_ONE_NZ(a, T) (T *)arena_push((a), sizeof(T), true)
#define PUSH_MANY(a, T, n) (T *)arena_push((a), sizeof(T) * (n), false)
#define PUSH_MANY_NZ(a, T, n) (T *)arena_push((a), sizeof(T) * (n), true)

struct Temp_Arena {
    Arena *arena;
    isize start_pos;
};

BASE_DEF Temp_Arena begin_temp_arena(Arena *a);
BASE_DEF void end_temp_arena(Temp_Arena temp);

#define SCRATCH_POOL 2
#define SCRATCH_RESERVE_SIZE (MiB(64))
#define SCRATCH_COMMIT_SIZE (MiB(1))

extern THREAD_LOCAL Arena *scratch_pool[SCRATCH_POOL];

BASE_DEF Temp_Arena acquire_scratch_arena(Arena **conflicts = NULL, i32 num_conflicts = 0);
BASE_DEF void release_scratch_arena(Temp_Arena scratch);

// Arrays

template <typename T>
struct Array {
    T *data;
    isize len;
    isize cap;

    Arena *arena;

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
void array_init(Array<T> *arr, isize initial_len, isize initial_cap, Arena *arena = NULL);

template <typename T>
void array_init(Array<T> *arr, isize initial_len, Arena *arena = NULL);

template <typename T>
void array_init(Array<T> *arr, Arena *arena = NULL);

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

inline String string_make(u8 *str, isize len);

inline String string_empty();

inline String string_from_cstr(const char *cstr);
inline const char *string_to_cstr(Arena *arena, const String &s);

inline bool string_eq(const String &a, const String &b);
inline bool string_ne(const String &a, const String &b);
inline bool string_lt(const String &a, const String &b);
inline bool string_gt(const String &a, const String &b);
inline bool string_le(const String &a, const String &b);
inline bool string_ge(const String &a, const String &b);

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

// Byte utils
inline bool byte_is_lower(u8 c);
inline bool byte_is_upper(u8 c);
inline bool byte_is_alpha(u8 c);
inline bool byte_is_digit(u8 c);
inline bool byte_is_alnum(u8 c);
inline bool byte_is_space(u8 c);
inline bool byte_in_set(u8 c, const String &strset);
inline u8 byte_to_lower(u8 c);
inline u8 byte_to_upper(u8 c);

// Non-allocating
BASE_DEF i32 string_compare(const String &a, const String &b);
BASE_DEF bool string_contains(const String &s, const String &substr);
BASE_DEF bool string_contains_byte(const String &s, u8 c);
BASE_DEF String string_cut_prefix(const String &s, const String &prefix);
BASE_DEF String string_cut_suffix(const String &s, const String &suffix);
BASE_DEF bool string_has_prefix(const String &s, const String &prefix);
BASE_DEF bool string_has_suffix(const String &s, const String &suffix);
BASE_DEF isize string_index(const String &s, const String &substr);
BASE_DEF isize string_index_byte(const String &s, u8 c);
BASE_DEF isize string_last_index(const String &s, const String &substr);
BASE_DEF isize string_last_index_byte(const String &s, u8 c);
BASE_DEF String string_trim(const String &s, const String &cutset);
BASE_DEF String string_trim_left(const String &s, const String &cutset);
BASE_DEF String string_trim_right(const String &s, const String &cutset);
BASE_DEF String string_trim_space(const String &s);
BASE_DEF String string_trim_prefix(const String &s, const String &prefix);
BASE_DEF String string_trim_suffix(const String &s, const String &suffix);

// Allocation-based
BASE_DEF String string_to_lower(Arena *arena, const String &s);
BASE_DEF String string_to_upper(Arena *arena, const String &s);
BASE_DEF String string_clone(Arena *arena, const String &s);
BASE_DEF String string_concat(Arena *arena, const String &a, const String &b);
BASE_DEF String string_join(Arena *arena, const Array<String> &elems, const String &sep);
BASE_DEF Array<String> string_split(Arena *arena, const String &s, const String &sep);
BASE_DEF String string_replace(Arena *arena, const String &s, const String &oldstr, const String &newstr);

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
    Table_Entry<K, V> *entries;
    isize cap;
    isize len;

    Arena *arena;
};

inline u32 table_hash(u64 x);

template <typename T>
inline u32 table_hash(T *ptr);

inline u32 table_hash(const String &s);

template <typename K, typename V>
void table_init(Table<K, V> *t, isize cap = 64, Arena *arena = NULL);

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
void table_resize(Table<K, V> *t, isize new_cap, Arena *arena);

template <typename K, typename V>
void table_clear(Table<K, V> *t);

//
// IMPLEMENTATION
//

#include <stdlib.h>

#if OS_WINDOWS
#include <windows.h>
#endif

// Memory

inline void *mem_alloc(isize size) {
    return malloc(size);
}

inline void *mem_realloc(void *ptr, isize new_size) {
    return realloc(ptr, new_size);
}

inline void *mem_copy(void *dest, const void *src, isize num_bytes) {
    return memcpy(dest, src, num_bytes);
}

inline void mem_free(void *ptr) {
    free(ptr);
}

inline void mem_set(void *dest, i32 ch, isize count) {
    memset(dest, ch, count);
}

inline i32 mem_compare(const void *a, const void *b, isize n) {
    return memcmp(a, b, n);
}

#ifdef BASE_IMPLEMENTATION

#if OS_WINDOWS

BASE_DEF isize mem_page_size() {
    local_persist isize result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwPageSize;
    }
    return result;
}

BASE_DEF isize mem_granularity() {
    local_persist isize result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwAllocationGranularity;
    }
    return result;
}

BASE_DEF void *mem_reserve(isize size) {
    size = ALIGN_UP(size, mem_page_size());
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

BASE_DEF bool mem_commit(void *ptr, isize size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    void *ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(ret == ptr);
    return ret != NULL;
}

BASE_DEF bool mem_decommit(void *ptr, isize size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    return VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

BASE_DEF bool mem_release(void *ptr) {
    ASSERT(ptr);
    return VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}

#endif

#endif // BASE_IMPLEMENTATION

// Arena

#ifdef BASE_IMPLEMENTATION

BASE_DEF Arena *arena_create(isize reserve_size, isize commit_size) {
    isize page_size = mem_page_size();
    isize gran = mem_granularity();
    reserve_size = ALIGN_UP(reserve_size, gran);
    commit_size = ALIGN_UP(commit_size, page_size);

    Arena *a = (Arena *)mem_reserve(reserve_size);
    ASSERT(mem_commit(a, commit_size) == true);

    a->reserve_size = reserve_size;
    a->commit_size = commit_size;
    a->pos = ARENA_BASE_POS;
    a->commit_pos = commit_size;
    return a;
}

BASE_DEF void arena_destroy(Arena *a) {
    mem_release(a);
}

BASE_DEF void *arena_push(Arena *a, isize size, bool non_zero) {
    isize pos_aligned = ALIGN_UP(a->pos, ARENA_ALIGN);
    isize new_pos = pos_aligned + size;

    if (new_pos > a->reserve_size) return NULL;

    if (new_pos > a->commit_pos) {
        isize new_commit_pos = new_pos;
        new_commit_pos += a->commit_size - 1;
        new_commit_pos -= new_commit_pos % a->commit_size;
        new_commit_pos = MIN(new_commit_pos, a->reserve_size);

        u8 *mem = (u8 *)a + a->commit_pos;
        isize commit_size = new_commit_pos - a->commit_pos;
        ASSERT(mem_commit(mem, commit_size) == true);

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

BASE_DEF Temp_Arena begin_temp_arena(Arena *a) {
    Temp_Arena temp = {};
    temp.arena = a;
    temp.start_pos = a->pos;
    return temp;
}

BASE_DEF void end_temp_arena(Temp_Arena temp) {
    arena_pop_to(temp.arena, temp.start_pos);
}

THREAD_LOCAL Arena *scratch_pool[SCRATCH_POOL] = { NULL, NULL };

BASE_DEF Temp_Arena acquire_scratch_arena(Arena **conflicts, i32 num_conflicts) {
    isize scratch_index = -1;

    for (isize i = 0; i < SCRATCH_POOL; i++) {
        bool found = false;

        for (i32 j = 0; j < num_conflicts; j++) {
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
    return begin_temp_arena(*selected);
}

BASE_DEF void release_scratch_arena(Temp_Arena scratch) {
    end_temp_arena(scratch);
}

#endif // BASE_IMPLEMENTATION

// Arrays

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
void array_init(Array<T> *arr, isize initial_len, isize initial_cap, Arena *arena) {
    ASSERT(initial_len >= 0 && initial_cap >= 0);

    arr->data = NULL;
    arr->len = 0;
    arr->cap = 0;
    arr->arena = arena;

    initial_cap = MAX(initial_len, initial_cap);

    if (initial_cap > 0) {
        if (arr->arena != NULL) {
            arr->data = PUSH_MANY(arena, T, initial_cap);
        } else {
            isize size = sizeof(T) * initial_cap;
            arr->data = (T *)mem_alloc(size);
            mem_set(arr->data, 0, size);
        }
        ASSERT(arr->data != NULL);
        arr->cap = initial_cap;
        arr->len = initial_len;
    }
}

template <typename T>
void array_init(Array<T> *arr, Arena *arena) {
    array_init(arr, 0, 0, arena);
}

template <typename T>
void array_init(Array<T> *arr, isize initial_len, Arena *arena) {
    array_init(arr, initial_len, initial_len, arena);
}

template <typename T>
void array_free(Array<T> *arr) {
    if (arr->data && !arr->arena) {
        mem_free(arr->data);
    }

    arr->data = NULL;
    arr->len = 0;
    arr->cap = 0;
    arr->arena = NULL;
}

template <typename T>
void array_reserve(Array<T> *arr, isize new_cap) {
    if (new_cap <= arr->cap) return;

    if (arr->arena) {
        if (array_can_grow_in_place(arr)) {
            isize extra = new_cap - arr->cap;
            void *ptr = (void *)PUSH_MANY(arr->arena, T, extra);
            ASSERT(ptr != NULL);
            arr->cap = new_cap;
        } else { // fallback: alloc + copy
            T *new_data = PUSH_MANY(arr->arena, T, new_cap);
            ASSERT(new_data != NULL);

            if (arr->data) {
                mem_copy(new_data, arr->data, sizeof(T) * arr->len);
            }

            arr->data = new_data;
            arr->cap = new_cap;
        }
    } else {
        isize new_size = new_cap * sizeof(T);

        if (arr->data) {
            void *ptr = mem_realloc(arr->data, new_size);
            ASSERT(ptr != NULL);
            arr->data = (T *)ptr;
        } else {
            arr->data = (T *)mem_alloc(new_size);
            ASSERT(arr->data != NULL);
        }

        arr->cap = new_cap;
    }
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

inline String string_make(u8 *str, isize len) {
    ASSERT(len >= 0);
    return String{ str, len };
}

inline String string_empty() {
    return string_make((u8 *)NULL, 0);
}

inline String string_from_cstr(const char *cstr) {
    return string_make((u8 *)cstr, (isize)strlen(cstr));
}

inline const char *string_to_cstr(Arena *arena, const String &s) {
    char *buf = (char *)arena_push(arena, s.len + 1);
    mem_copy(buf, s.data, s.len);
    buf[s.len] = 0;
    return buf;
}

inline bool string_eq(const String &a, const String &b) { return a.len == b.len && mem_compare(a.data, b.data, a.len) == 0; }
inline bool string_ne(const String &a, const String &b) { return !string_eq(a,b); }
inline bool string_lt(const String &a, const String &b) { return string_compare(a,b) < 0; }
inline bool string_gt(const String &a, const String &b) { return string_compare(a,b) > 0; }
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

template <> inline bool operator == (const String &a, const char (&b)[1]) { return a.len == 0; }
template <> inline bool operator != (const String &a, const char (&b)[1]) { return a.len != 0; }

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

#ifdef BASE_IMPLEMENTATION

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

BASE_DEF String string_to_lower(Arena *arena, const String &s) {
    u8 *data = PUSH_MANY(arena, u8, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = byte_to_lower(s.data[i]);
    return string_make(data, s.len);
}

BASE_DEF String string_to_upper(Arena *arena, const String &s) {
    u8 *data = PUSH_MANY(arena, u8, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = byte_to_upper(s.data[i]);
    return string_make(data, s.len);
}

BASE_DEF String string_clone(Arena *arena, const String &s) {
    u8 *data = PUSH_MANY(arena, u8, s.len);
    mem_copy(data, s.data, s.len);
    return string_make(data, s.len);
}

BASE_DEF String string_concat(Arena *arena, const String &a, const String &b) {
    isize len = a.len + b.len;
    u8 *data = PUSH_MANY(arena, u8, len);

    mem_copy(data, a.data, a.len);
    mem_copy(data + a.len, b.data, b.len);

    return string_make(data, len);
}

BASE_DEF String string_join(Arena *arena, const Array<String> &elems, const String &sep) {
    if (elems.len == 0) return string_empty();

    // compute total length
    isize total = 0;
    for (isize i = 0; i < elems.len; i++) {
        total += elems[i].len;
    }
    total += sep.len * (elems.len - 1);

    u8 *data = PUSH_MANY(arena, u8, total);

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

BASE_DEF Array<String> string_split(Arena *arena, const String &s, const String &sep) {
    Array<String> result;
    array_init(&result, arena);

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

BASE_DEF String string_replace(Arena *arena,
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

    u8 *data = PUSH_MANY(arena, u8, new_len);

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
void table_init(Table<K, V> *t, isize cap, Arena *arena) {
    t->cap = cap;
    t->len = 0;
    t->arena = arena;

    if (arena) {
        t->entries = (Table_Entry<K, V> *)
            arena_push(arena, sizeof(Table_Entry<K, V>) * cap);
    } else {
        t->entries = (Table_Entry<K, V> *)
            mem_alloc(sizeof(Table_Entry<K, V>) * cap);
    }

    for (isize i = 0; i < cap; i++) {
        t->entries[i].state = TABLE_SLOT_EMPTY;
    }
}

template <typename K, typename V>
void table_free(Table<K, V> *t) {
    if (!t->arena && t->entries) {
        mem_free(t->entries);
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
        table_resize(t, t->cap * 2, t->arena);
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
void table_resize(Table<K, V> *t, isize new_cap, Arena *arena) {
    Table_Entry<K, V> *old_entries = t->entries;
    isize old_cap = t->cap;

    Table_Entry<K, V> *new_entries;
    if (t->arena) {
        new_entries = (Table_Entry<K, V> *)
            arena_push(t->arena, sizeof(Table_Entry<K, V>) * new_cap);
    } else {
        new_entries = (Table_Entry<K, V> *)
            mem_alloc(sizeof(Table_Entry<K, V>) * new_cap);
    }

    t->entries = new_entries;
    t->cap = new_cap;
    t->len = 0;

    for (isize i = 0; i < new_cap; i++) {
        new_entries[i].state = TABLE_SLOT_EMPTY;
    }

    for (isize i = 0; i < old_cap; i++) {
        auto *e = &old_entries[i];
        if (e->state == TABLE_SLOT_OCCUPIED) {
            table_set(t, e->key, e->value);
        }
    }

    if (!t->arena) mem_free(old_entries);
}

template <typename K, typename V>
void table_clear(Table<K, V> *t) {
    for (isize i = 0; i < t->cap; i++) {
        t->entries[i].state = TABLE_SLOT_EMPTY;
    }

    t->len = 0;
}

#endif // BASE_H
