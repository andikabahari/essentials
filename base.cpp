#include "base.h"

#include <stdlib.h>

C_LINKAGE_BEGIN

// Memory

static Alloc_Proc  *mem_alloc_proc  = &malloc;
static Resize_Proc *mem_resize_proc = &realloc;
static Free_Proc   *mem_free_proc   = &free;

void mem_set_procs(Alloc_Proc *alloc_proc, Resize_Proc *resize_proc, Free_Proc *free_proc) {
    mem_alloc_proc  = alloc_proc;
    mem_resize_proc = resize_proc;
    mem_free_proc   = free_proc;
}

void *mem_alloc(isize sz) {
    return mem_alloc_proc(sz);
}

void *mem_resize(void *ptr, isize newsz) {
    return mem_resize_proc(ptr, newsz);
}

void mem_free(void *ptr) {
    mem_free_proc(ptr);
}

// Arena

Arena *arena_create(isize reserve_size, isize commit_size) {
    reserve_size = ALIGN_UP(reserve_size, ARENA_ALIGN);
    commit_size  = ALIGN_UP(commit_size, ARENA_ALIGN);

    Arena *a = (Arena *)mem_alloc(reserve_size);
    ASSERT(a);

    a->reserve_size = reserve_size;
    a->commit_size  = commit_size;
    a->pos          = ARENA_BASE_POS;
    a->commit_pos   = commit_size;

    return a;
}

void arena_destroy(Arena *a) {
    mem_free(a);
}

void *arena_push(Arena *a, isize size, bool non_zero) {
    isize pos_aligned = ALIGN_UP(a->pos, ARENA_ALIGN);
    isize new_pos = pos_aligned + size;

    if (new_pos > a->reserve_size) return NULL;

    if (new_pos > a->commit_pos) {
        isize new_pos_aligned = ALIGN_UP(new_pos, a->commit_size);
        isize new_commit_pos  = MIN(new_pos_aligned, a->reserve_size);
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

Arena_Temp arena_begin_temp(Arena *a) {
    Arena_Temp temp;
    temp.arena = a;
    temp.start_pos = a->pos;
    return temp;
}

void arena_end_temp(Arena_Temp temp) {
    arena_pop_to(temp.arena, temp.start_pos);
}

static THREAD_LOCAL Arena *arena_scratch_pool[ARENA_SCRATCH_POOL] = { 0 };

Arena_Temp arena_begin_scratch(Arena **conflicts, i32 num_conflicts) {
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
        Arena_Temp t = { 0 };
        return t;
    }

    Arena **selected = &arena_scratch_pool[scratch_index];
    if (*selected == NULL) {
        *selected = arena_create(ARENA_SCRATCH_RESERVE_SIZE, ARENA_SCRATCH_COMMIT_SIZE);
        ASSERT(*selected);
    }
    return arena_begin_temp(*selected);
}

void arena_end_scratch(Arena_Temp scratch) {
    arena_end_temp(scratch);
}

// Custom allocation

void *allocator_alloc(Allocator a, isize sz) {
    return a.proc(a.data, ALLOCATION_ALLOC, sz, DEFAULT_MEMORY_ALIGNMENT, NULL, 0);
}

void *allocator_resize(Allocator a, void *ptr, isize oldsz, isize newsz) {
    return a.proc(a.data, ALLOCATION_RESIZE, newsz, DEFAULT_MEMORY_ALIGNMENT, ptr, oldsz);
}

void allocator_free(Allocator a, void *ptr) {
    if (ptr) {
        a.proc(a.data, ALLOCATION_FREE, 0, 0, ptr, 0);
    }
}

void allocator_free_all(Allocator a) {
    a.proc(a.data, ALLOCATION_FREE_ALL, 0, 0, NULL, 0);
}

Allocator heap_allocator(void) {
    Allocator a;
    a.proc = heap_allocator_proc;
    a.data = NULL;
    return a;
}

ALLOCATOR_PROC(heap_allocator_proc) {
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

Allocator arena_allocator(Arena *arena) {
    Allocator a;
    a.proc = arena_allocator_proc;
    a.data = arena;
    return a;
}

ALLOCATOR_PROC(arena_allocator_proc) {
    UNUSED(alignment);
    UNUSED(oldmem);
    UNUSED(oldsz);

    Arena *a = (Arena *)alloc_data;
    void *ptr = NULL;

    switch (alloc_mode) {
        case ALLOCATION_ALLOC:
            ptr = arena_push(a, newsz, false);
            break;
        case ALLOCATION_RESIZE:
            // TODO: should I handle this case?
            break;
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

static Log_Proc *log_proc = &log_default_proc;

void log_set_proc(Log_Proc *proc) {
    log_proc = proc;
}

Log_Proc *log_get_proc(void) {
    return log_proc;
}

static Log_Level log_min_level = LOG_INFO;

void log_set_minimum_level(Log_Level level) {
    log_min_level = level;
}

LOG_PROC(log_default_proc) {
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

LOG_PROC(log_empty_proc) {
    UNUSED(level);
    UNUSED(fmt);
    UNUSED(args);
}

void log_printf(Log_Level level, const char *fmt, ...) {
    if (level < log_min_level) return;

    va_list args;
    va_start(args, fmt);
    log_proc(level, fmt, args);
    va_end(args);
}

C_LINKAGE_END

#if LANG_CPP

// Strings

bool byte_is_lower(u8 c) {
    return 'a' <= c && c <= 'z';
}

bool byte_is_upper(u8 c) {
    return 'A' <= c && c <= 'Z';
}

bool byte_is_alpha(u8 c) {
    return byte_is_lower(c) || byte_is_upper(c);
}

bool byte_is_digit(u8 c) {
    return '0' <= c && c <= '9';
}

bool byte_is_alnum(u8 c) {
    return byte_is_alpha(c) || byte_is_digit(c);
}

bool byte_is_space(u8 c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool byte_in_set(u8 c, const String &strset) {
    for (isize i = 0; i < strset.len; i++) {
        if (strset.data[i] == c) return true;
    }
    return false;
}

u8 byte_to_lower(u8 c) {
    return byte_is_upper(c) ? c + 32 : c;
}

u8 byte_to_upper(u8 c) {
    return byte_is_lower(c) ? c - 32 : c;
}

String string_make(u8 *str, isize len) {
    ASSERT(len >= 0);
    return String{ str, len };
}

String string_empty() {
    return string_make((u8 *)NULL, 0);
}

String string_from_cstr(const char *cstr) {
    return string_make((u8 *)cstr, (isize)strlen(cstr));
}

const char *string_to_cstr(const Allocator &al, const String &s) {
    char *buf = (char *)allocator_alloc(al, s.len + 1);
    mem_copy(buf, s.data, s.len);
    buf[s.len] = 0;
    return buf;
}

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
        return string_make(s.data + prefix.len, s.len - prefix.len);
    }
    return s;
}

String string_cut_suffix(const String &s, const String &suffix) {
    if (string_has_suffix(s, suffix)) {
        return string_make(s.data, s.len - suffix.len);
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
    return string_make(s.data + i, s.len - i);
}

String string_trim_right(const String &s, const String &cutset) {
    isize end = s.len;
    while (end > 0 && byte_in_set(s.data[end - 1], cutset)) end--;
    return string_make(s.data, end);
}

String string_trim_space(const String &s) {
    isize start = 0;
    while (start < s.len && byte_is_space(s.data[start])) start++;

    isize end = s.len;
    while (end > start && byte_is_space(s.data[end - 1])) end--;

    return string_make(s.data + start, end - start);
}

String string_trim(const String &s, const String &cutset) {
    return string_trim_right(string_trim_left(s, cutset), cutset);
}

String string_trim_prefix(const String &s, const String &prefix) {
    if (string_has_prefix(s, prefix)) {
        return string_make(s.data + prefix.len, s.len - prefix.len);
    }
    return s;
}

String string_trim_suffix(const String &s, const String &suffix) {
    if (string_has_suffix(s, suffix)) {
        return string_make(s.data, s.len - suffix.len);
    }
    return s;
}

String string_to_lower(const Allocator &al, const String &s) {
    u8 *data = (u8 *)allocator_alloc(al, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = byte_to_lower(s.data[i]);
    return string_make(data, s.len);
}

String string_to_upper(const Allocator &al, const String &s) {
    u8 *data = (u8 *)allocator_alloc(al, s.len);
    for (isize i = 0; i < s.len; i++) data[i] = byte_to_upper(s.data[i]);
    return string_make(data, s.len);
}

String string_clone(const Allocator &al, const String &s) {
    u8 *data = (u8 *)allocator_alloc(al, s.len);
    mem_copy(data, s.data, s.len);
    return string_make(data, s.len);
}

String string_concat(const Allocator &al, const String &a, const String &b) {
    isize len = a.len + b.len;
    u8 *data = (u8 *)allocator_alloc(al, len);

    mem_copy(data, a.data, a.len);
    mem_copy(data + a.len, b.data, b.len);

    return string_make(data, len);
}

String string_join(const Allocator &al, const Array<String> &elems, const String &sep) {
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

Array<String> string_split(const Allocator &al, const String &s, const String &sep) {
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

String string_replace(const Allocator &al,
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

// Hash tables

// Murmur
u32 table_hash(u64 x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return (u32)x;
}

// FNV-1a
u32 table_hash(const String &s) {
    u32 h = 2166136261u;
    for (isize i = 0; i < s.len; i++) {
        h ^= (u8)s.data[i];
        h *= 16777619u;
    }
    return h;
}

#endif
