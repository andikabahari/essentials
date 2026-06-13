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

typedef u16 Gfx_Index;
#define GFX_INDEX_ELEMENT_SIZE SDL_GPU_INDEXELEMENTSIZE_16BIT

#define GFX_NUM_VERTICES_PER_QUAD       4
#define GFX_NUM_INDICES_PER_QUAD        6
#define GFX_MAX_QUADS_PER_BATCH         8192
#define GFX_MAX_QUAD_VERTICES_PER_BATCH GFX_NUM_VERTICES_PER_QUAD * GFX_MAX_QUADS_PER_BATCH
#define GFX_MAX_QUAD_INDICES_PER_BATCH  GFX_NUM_INDICES_PER_QUAD * GFX_MAX_QUADS_PER_BATCH

struct Gfx_Quad_Vertex {
    Vec2 position;
    Vec2 texcoord;
    Gfx_Color color;
};

struct Gfx_Quad_Batch {
    SDL_GPUGraphicsPipeline *pipeline;

    SDL_GPUBufferRegion vertex_region;
    SDL_GPUBufferRegion index_region;
    SDL_GPUTransferBuffer *upload_buffer;
    u8 *upload_ptr;

    SDL_GPUTexture *white_texture;
    SDL_GPUSampler *white_sampler;

    u32 push_count;
};

struct Gfx_State {
    Allocator allocator;

    SDL_Window    *window;
    SDL_GPUDevice *device;

    Gfx_Color clear_color;

    bool drawing;
    Gfx_Quad_Batch quad_batch;
};

static Gfx_State *gfx = NULL;

#define GFX_SHADER_FORMAT SDL_GPU_SHADERFORMAT_SPIRV
static SDL_GPUShader *gfx_load_shader(const String &file);

static void gfx_init_quad_batch(void);
static void gfx_free_quad_batch(void);
static void gfx_flush_quad_batch(void);
static void gfx_render_quad_batch(void);

void gfx_init(const Allocator &a, SDL_Window *window);
void gfx_quit(void);

void gfx_set_clear_color(Gfx_Color color);

void gfx_begin_drawing(void);
void gfx_end_drawing(void);

void gfx_draw_rectangle(f32 x, f32 y, f32 w, f32 h, Gfx_Color color);

#endif // GFX_H
