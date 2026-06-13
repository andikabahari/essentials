#include "../base.h"
#include "../gfx.h"

LOG_PROC(sdl_log_proc) {
    SDL_LogPriority priority;

    switch (level) {
        case LOG_INFO:    priority = SDL_LOG_PRIORITY_INFO;    break;
        case LOG_WARNING: priority = SDL_LOG_PRIORITY_WARN;    break;
        case LOG_ERROR:   priority = SDL_LOG_PRIORITY_ERROR;   break;
        default:          priority = SDL_LOG_PRIORITY_INVALID; break;
    }

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, priority, fmt, args);
}

int main(void) {
    mem_set_procs(&SDL_malloc, &SDL_realloc, &SDL_free);
    log_set_proc(&sdl_log_proc);
    #if !BUILD_DEBUG
        log_set_minimum_level(LOG_WARNING);
    #endif

    ASSERT(SDL_Init(SDL_INIT_VIDEO));
    defer (SDL_Quit());

    SDL_Window *window = SDL_CreateWindow("sample_triangle", 800, 600, 0);
    ASSERT(window);
    defer (SDL_DestroyWindow(window));

    gfx_init(heap_allocator(), window);
    defer (gfx_quit());

    u32 frame_per_seconds = 0;

    u64 last_time = 0;
    u64 current_time;

    bool keep_running = true;
    SDL_Event event;
    while (keep_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                keep_running = false;
            }
        }

        gfx_begin_drawing();

        for_count(i, GFX_MAX_QUADS_PER_BATCH) {
            gfx_draw_rectangle(0, 0, 10, 10, GFX_RED);
        }

        gfx_end_drawing();

        frame_per_seconds += 1;

        current_time = SDL_GetTicks();
        if (current_time > last_time + 1000) {
            last_time = current_time;

            printf("FPS: %llu\n", frame_per_seconds);
            frame_per_seconds = 0;
        }
    }

    return 0;
}
