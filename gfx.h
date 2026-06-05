#ifndef GFX_H
#define GFX_H

#include "base.h"
#include "linalg.h"

#include <SDL3/SDL.h>

struct Gfx_Color { u8 r, g, b, a; };

static const Gfx_Color GFX_WHITE   = { 255, 255, 255, 255 };
static const Gfx_Color GFX_BLACK   = {   0,   0,   0, 255 };
static const Gfx_Color GFX_RED     = { 255,   0,   0, 255 };
static const Gfx_Color GFX_GREEN   = {   0, 255,   0, 255 };
static const Gfx_Color GFX_BLUE    = {   0,   0, 255, 255 };
static const Gfx_Color GFX_YELLOW  = { 255, 255,   0, 255 };

struct Gfx_Vertex2D {
    Vec2 position;
    Vec2 texcoord;
    Gfx_Color color;
};

#define GFX_RENDER2D_MAX_VERTICES 65536

struct Gfx_Render2D {
    SDL_GPUGraphicsPipeline *pipeline;
    SDL_GPUBuffer *vertex_buffer;
    SDL_GPUBuffer *index_buffer;
};

// TODO: render 3D

struct Gfx_State {
    Allocator allocator;

    SDL_Window    *window;
    SDL_GPUDevice *device;
    i32            window_width;
    i32            window_height;

    Gfx_Render2D _2d;
};

static Gfx_State *gfx_state = NULL;

#define GFX_SHADER_FORMAT SDL_GPU_SHADERFORMAT_SPIRV
static SDL_GPUShader *gfx_load_shader(const String &file);

void gfx_init(const Allocator &a, SDL_Window *window);
void gfx_quit();
void gfx_draw(Gfx_Color clear_color);

#endif // GFX_H
