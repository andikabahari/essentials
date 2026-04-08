#ifndef BASE_H
#define BASE_H

/*

Critical rules:
   1. Templates
      - Always in header
      - Never inside `#ifdef BASE_IMPLEMENTATION`
   2. Non-template functions
      - Declaration in header
      - Definition ONLY inside `#ifdef BASE_IMPLEMENTATION`
   3. Exactly ONE implementation:
      - One C/C++ file with `#define BASE_IMPLEMENTATION`
      - OR compile this file into `.lib`

*/

//
// DECLARATION
//

#define local_persist static
#define internal static
#define global_var static

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

inline void *mem_alloc(isize size);
inline void *mem_realloc(void *ptr, isize new_size);
inline void *mem_copy(void *dest, const void *src, isize num_bytes);
inline void mem_free(void *ptr);
inline void mem_set(void *dest, i32 ch, isize count);
inline i32  mem_compare(const void *a, const void *b, isize n);

isize mem_page_size();
isize mem_granularity();

void *mem_reserve(isize size);
bool mem_commit(void *ptr, isize size);
bool mem_decommit(void *ptr, isize size);
bool mem_release(void *ptr);

// Arena

#define ARENA_BASE_POS ALIGN_UP(sizeof(Arena), ARENA_ALIGN)
#define ARENA_ALIGN (sizeof(void *))

struct Arena {
    isize reserve_size;
    isize commit_size;
    isize pos;
    isize commit_pos;
};

Arena *arena_create(isize reserve_size, isize commit_size);
void arena_destroy(Arena *a);
void *arena_push(Arena *a, isize size, bool non_zero);
void arena_pop(Arena *a, isize size);
void arena_pop_to(Arena *a, isize pos);
void arena_clear(Arena *a);

#define PUSH_STRUCT(a, T) (T *)arena_push((a), sizeof(T), false)
#define PUSH_STRUCT_NZ(a, T) (T *)arena_push((a), sizeof(T), true)
#define PUSH_ARRAY(a, T, n) (T *)arena_push((a), sizeof(T) * (n), false)
#define PUSH_ARRAY_NZ(a, T, n) (T *)arena_push((a), sizeof(T) * (n), true)

struct Temp_Arena {
    Arena *arena;
    isize start_pos;
};

Temp_Arena begin_temp_arena(Arena *a);
void end_temp_arena(Temp_Arena temp);

#define SCRATCH_POOL 2
#define SCRATCH_RESERVE_SIZE (MiB(64))
#define SCRATCH_COMMIT_SIZE (MiB(1))

extern THREAD_LOCAL Arena *scratch_pool[SCRATCH_POOL];

Temp_Arena get_scratch_arena(Arena **conflicts, i32 num_conflicts);
void release_scratch_arena(Temp_Arena scratch);

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
void array_init(Array<T> *arr, isize initial_cap = 0);

template <typename T>
void array_init(Array<T> *arr, Arena *arena, isize initial_cap = 0);

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

/*

Notes:
    String -> non-owning view
    String_Buffer -> owning, mutable
    Slice / view -> no alloc
    Concat / join -> alloc in arena
    Split -> alloc array + substring (views or copies)
    String doesn't store null character

*/

struct String {
    u8 *data;
    isize len;

    u8 &operator[](isize index) {
        ASSERT(0 <= index && index < len);
        return data[index];
    }

    const u8 &operator[](isize index) const {
        ASSERT(0 <= index && index < len);
        return data[index];
    }
};

// Example: String s = LIT("Hello!");
#define LIT(s) String{(u8 *)(s), sizeof(s)-1}

// Example: printf("%.*s\n", FMT(s));
#define FMT(s) (int)(s).len, (const char *)(s).data

inline String string_from_cstring(const char *cstr);
inline const char *string_to_cstring(Arena *arena, const String &s);

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

// Utils
inline u8 to_lower(u8 c);
inline u8 to_upper(u8 c);
inline bool is_space(u8 c);
inline bool byte_in_set(u8 c, const String &strset);

// Non-allocating
i32 string_compare(const String &a, const String &b);
bool string_contains(const String &s, const String &substr);
bool string_contains_byte(const String &s, u8 c);
String string_cut_prefix(const String &s, const String &prefix);
String string_cut_suffix(const String &s, const String &suffix);
bool string_has_prefix(const String &s, const String &prefix);
bool string_has_suffix(const String &s, const String &suffix);
isize string_index(const String &s, const String &substr);
isize string_index_byte(const String &s, u8 c);
isize string_last_index(const String &s, const String &substr);
isize string_last_index_byte(const String &s, u8 c);
String string_trim(const String &s, const String &cutset);
String string_trim_left(const String &s, const String &cutset);
String string_trim_right(const String &s, const String &cutset);
String string_trim_space(const String &s);
String string_trim_prefix(const String &s, const String &prefix);
String string_trim_suffix(const String &s, const String &suffix);

// Allocation-based
String string_to_lower(Arena *arena, const String &s);
String string_to_upper(Arena *arena, const String &s);
String string_clone(Arena *arena, const String &s);
String string_concat(Arena *arena, const String &a, const String &b);
String string_join(Arena *arena, const Array<String> &elems, const String &sep);
Array<String> string_split(Arena *arena, const String &s, const String &sep);
String string_replace(Arena *arena, const String &s, const String &oldstr, const String &newstr);

//
// DEFINITION
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

isize mem_page_size() {
    local_persist isize result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwPageSize;
    }
    return result;
}

isize mem_granularity() {
    local_persist isize result = 0;
    if (result == 0) {
        SYSTEM_INFO sysinfo = {};
        GetSystemInfo(&sysinfo);
        result = sysinfo.dwAllocationGranularity;
    }
    return result;
}

void *mem_reserve(isize size) {
    size = ALIGN_UP(size, mem_page_size());
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

bool mem_commit(void *ptr, isize size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    void *ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(ret == ptr);
    return ret != NULL;
}

bool mem_decommit(void *ptr, isize size) {
    ASSERT(ptr);
    size = ALIGN_UP(size, mem_page_size());
    return VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

bool mem_release(void *ptr) {
    ASSERT(ptr);
    return VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}

#endif

#endif // BASE_IMPLEMENTATION

// Arena

#ifdef BASE_IMPLEMENTATION

Arena *arena_create(isize reserve_size, isize commit_size) {
    isize page_size = mem_page_size();
    isize gran = mem_granularity();
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

void arena_destroy(Arena *a) {
    mem_release(a);
}

void *arena_push(Arena *a, isize size, bool non_zero) {
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
        if (!mem_commit(mem, commit_size)) return NULL;

        a->commit_pos = new_commit_pos;
    }

    a->pos = new_pos;

    u8 *out = (u8 *)a + pos_aligned;
    if (!non_zero) mem_set(out, 0, size);
    return out;
}

void arena_pop(Arena *a, isize size) {
    size = MIN(size, a->pos - ARENA_BASE_POS);
    a->pos -= size;
}

void arena_pop_to(Arena *a, isize pos) {
    isize size = pos < a->pos ? a->pos - pos : 0;
    arena_pop(a, size);
}

void arena_clear(Arena *a) {
    arena_pop_to(a, ARENA_BASE_POS);
}

Temp_Arena begin_temp_arena(Arena *a) {
    Temp_Arena temp = {};
    temp.arena = a;
    temp.start_pos = a->pos;
    return temp;
}

void end_temp_arena(Temp_Arena temp) {
    arena_pop_to(temp.arena, temp.start_pos);
}

THREAD_LOCAL Arena *scratch_pool[SCRATCH_POOL] = { NULL, NULL };

Temp_Arena get_scratch_arena(Arena **conflicts, i32 num_conflicts) {
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

void release_scratch_arena(Temp_Arena scratch) {
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
void array_init(Array<T> *arr, Arena *arena, isize initial_cap) {
    arr->data = NULL;
    arr->len = 0;
    arr->cap = 0;
    arr->arena = arena;

    if (initial_cap > 0) {
        if (arr->arena != NULL) {
            arr->data = PUSH_ARRAY(arena, T, initial_cap);
        } else {
            isize size = sizeof(T) * initial_cap;
            arr->data = (T *)mem_alloc(size);
        }
        ASSERT(arr->data != NULL);
        arr->cap = initial_cap;
    }
}

template <typename T>
void array_init(Array<T> *arr, isize initial_cap) {
    array_init(arr, NULL, initial_cap);
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
            void *ptr = (void *)PUSH_ARRAY(arr->arena, T, extra);
            ASSERT(ptr != NULL);
            arr->cap = new_cap;
        } else { // fallback: alloc + copy
            T *new_data = PUSH_ARRAY(arr->arena, T, new_cap);
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

inline String string_from_cstr(const char *cstr) {
    return String{(u8 *)cstr, (isize)strlen(cstr)};
}

inline const char *string_to_cstr(Arena *arena, const String &s) {
    char *buf = (char *)arena_push(arena, s.len + 1, false);
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

template <isize N> inline bool operator == (const String &a, const char (&b)[N]) { return string_eq(a, String{(u8 *)b, N-1}); }
template <isize N> inline bool operator != (const String &a, const char (&b)[N]) { return string_ne(a, String{(u8 *)b, N-1}); }
template <isize N> inline bool operator <  (const String &a, const char (&b)[N]) { return string_lt(a, String{(u8 *)b, N-1}); }
template <isize N> inline bool operator >  (const String &a, const char (&b)[N]) { return string_gt(a, String{(u8 *)b, N-1}); }
template <isize N> inline bool operator <= (const String &a, const char (&b)[N]) { return string_le(a, String{(u8 *)b, N-1}); }
template <isize N> inline bool operator >= (const String &a, const char (&b)[N]) { return string_ge(a, String{(u8 *)b, N-1}); }

template <> inline bool operator == (const String &a, const char (&b)[1]) { return a.len == 0; }
template <> inline bool operator != (const String &a, const char (&b)[1]) { return a.len != 0; }

inline u8 to_lower(u8 c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

inline u8 to_upper(u8 c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

inline bool is_space(u8 c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool byte_in_set(u8 c, const String &strset) {
    for (isize i = 0; i < strset.len; i++) {
        if (strset.data[i] == c) return true;
    }
    return false;
}

#ifdef BASE_IMPLEMENTATION

i32 string_compare(const String &a, const String &b) {
    isize n = MIN(a.len, b.len);

    i32 cmp = mem_compare(a.data, b.data, n);
    if (cmp != 0) return cmp;

    if (a.len < b.len) return -1;
    if (a.len > b.len) return 1;
    return 0;
}

bool string_contains(const String &s, const String &sub) {
    return string_index(s, sub) >= 0;
}

bool string_contains_byte(const String &s, u8 c) {
    return string_index_byte(s, c) >= 0;
}

String string_cut_prefix(const String &s, const String &prefix) {
    if (string_has_prefix(s, prefix)) {
        return String{ s.data + prefix.len, s.len - prefix.len };
    }
    return s;
}

String string_cut_suffix(const String &s, const String &suffix) {
    if (string_has_suffix(s, suffix)) {
        return String{ s.data, s.len - suffix.len };
    }
    return s;
}

bool string_has_prefix(const String &s, const String &prefix) {
    if (prefix.len > s.len) return false;
    return mem_compare(s.data, prefix.data, prefix.len) == 0;
}

bool string_has_suffix(const String &s, const String &suffix) {
    if (suffix.len > s.len) return false;
    return mem_compare(s.data + (s.len - suffix.len), suffix.data, suffix.len) == 0;
}

isize string_index(const String &s, const String &sub) {
    if (sub.len == 0) return 0;
    if (sub.len > s.len) return -1;

    for (isize i = 0; i <= s.len - sub.len; i++) {
        if (mem_compare(s.data + i, sub.data, sub.len) == 0) {
            return i;
        }
    }
    return -1;
}

isize string_index_byte(const String &s, u8 c) {
    for (isize i = 0; i < s.len; i++) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

isize string_last_index(const String &s, const String &sub) {
    if (sub.len == 0) return s.len;
    if (sub.len > s.len) return -1;

    for (isize i = s.len - sub.len; i >= 0; i--) {
        if (mem_compare(s.data + i, sub.data, sub.len) == 0) {
            return i;
        }
    }
    return -1;
}

isize string_last_index_byte(const String &s, u8 c) {
    for (isize i = s.len - 1; i >= 0; i--) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

String string_trim_left(const String &s, const String &cutset) {
    isize i = 0;
    while (i < s.len && byte_in_set(s.data[i], cutset)) i++;
    return String{ s.data + i, s.len - i };
}

String string_trim_right(const String &s, const String &cutset) {
    isize end = s.len;
    while (end > 0 && byte_in_set(s.data[end - 1], cutset)) end--;
    return String{ s.data, end };
}

String string_trim_space(const String &s) {
    isize start = 0;
    while (start < s.len && is_space(s.data[start])) start++;

    isize end = s.len;
    while (end > start && is_space(s.data[end - 1])) end--;

    return String{ s.data + start, end - start };
}

String string_trim(const String &s, const String &cutset) {
    return string_trim_right(string_trim_left(s, cutset), cutset);
}

String string_trim_prefix(const String &s, const String &prefix) {
    if (string_has_prefix(s, prefix)) {
        return String{ s.data + prefix.len, s.len - prefix.len };
    }
    return s;
}

String string_trim_suffix(const String &s, const String &suffix) {
    if (string_has_suffix(s, suffix)) {
        return String{ s.data, s.len - suffix.len };
    }
    return s;
}

String string_to_lower(Arena *arena, const String &s) {
    u8 *data = PUSH_ARRAY(arena, u8, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = to_lower(s.data[i]);
    return String{ data, s.len };
}

String string_to_upper(Arena *arena, const String &s) {
    u8 *data = PUSH_ARRAY(arena, u8, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = to_upper(s.data[i]);
    return String{ data, s.len };
}

String string_clone(Arena *arena, const String &s) {
    u8 *data = PUSH_ARRAY(arena, u8, s.len);
    mem_copy(data, s.data, s.len);
    return String{ data, s.len };
}

String string_concat(Arena *arena, const String &a, const String &b) {
    isize len = a.len + b.len;
    u8 *data = PUSH_ARRAY(arena, u8, len);

    mem_copy(data, a.data, a.len);
    mem_copy(data + a.len, b.data, b.len);

    return String{ data, len };
}

String string_join(Arena *arena, const Array<String> &elems, const String &sep) {
    if (elems.len == 0) return String{0,0};

    // compute total length
    isize total = 0;
    for (isize i = 0; i < elems.len; i++) {
        total += elems[i].len;
    }
    total += sep.len * (elems.len - 1);

    u8 *data = PUSH_ARRAY(arena, u8, total);

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

    return String{ data, total };
}

Array<String> string_split(Arena *arena, const String &s, const String &sep) {
    Array<String> result;
    array_init(&result, arena);

    if (sep.len == 0) {
        // split into bytes
        for (isize i = 0; i < s.len; i++) {
            array_add(&result, String{ s.data + i, 1 });
        }
        return result;
    }

    isize start = 0;

    while (start <= s.len) {
        isize idx = string_index(
            String{ s.data + start, s.len - start }, sep
        );

        if (idx < 0) {
            // last segment
            array_add(&result, String{
                s.data + start,
                s.len - start
            });
            break;
        }

        array_add(&result, String{
            s.data + start,
            idx
        });

        start += idx + sep.len;
    }

    return result;
}

String string_replace(
        Arena *arena,
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

    u8 *data = PUSH_ARRAY(arena, u8, new_len);

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

    return String{ data, new_len };
}

#endif // BASE_IMPLEMENTATION

#endif // BASE_H
