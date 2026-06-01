#ifndef GFX_H
#define GFX_H

#include "base.h"

#include <SDL3/SDL.h>

bool gfx_init(SDL_Window *window);
void gfx_quit();
void gfx_draw();

SDL_GPUShader *gfx_load_shader(const String &file);

#endif // GFX_H
