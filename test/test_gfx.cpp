TEST(test_gfx) {
    ASSERT(SDL_Init(SDL_INIT_VIDEO));
    defer(SDL_Quit());

    SDL_Window *window = SDL_CreateWindow("test_gfx", 800, 600, 0);
    ASSERT(window);
    defer(SDL_DestroyWindow(window));

    ASSERT(gfx_init(window));
    defer(gfx_quit());

    bool keep_running = true;
    SDL_Event event;
    while (keep_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                keep_running = false;
            }
        }

        gfx_draw();
    }
}
