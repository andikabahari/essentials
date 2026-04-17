#ifndef GFX_H
#define GFX_H

#include "base.h"

#pragma push_macro("internal")
#undef internal
#include <SDL3/SDL.h>
#pragma pop_macro("internal")

//
// DECLARATION
//

extern bool gfx_initted;
extern SDL_Window *gfx_window;
extern SDL_GPUDevice *gfx_device;
extern SDL_GPUGraphicsPipeline *gfx_pipeline;

#define GFX_SHADER_FORMAT (SDL_GPU_SHADERFORMAT_SPIRV)

IDEF bool gfx_init(SDL_Window *window);
IDEF void gfx_quit();
IDEF void gfx_draw();

//
// DEFINITION/IMPLEMENTATION
//

#ifdef GFX_IMPLEMENTATION

bool gfx_initted = false;
SDL_Window *gfx_window = NULL;
SDL_GPUDevice *gfx_device = NULL;
SDL_GPUGraphicsPipeline *gfx_pipeline = NULL;

IDEF bool gfx_init(SDL_Window *window) {
    gfx_window = window;
    gfx_device = SDL_CreateGPUDevice(GFX_SHADER_FORMAT, true, NULL);

    if (!gfx_device) {
        gfx_initted = false;
        return gfx_initted;
    }

    if (!SDL_ClaimWindowForGPUDevice(gfx_device, gfx_window)) {
        gfx_initted = false;
        return gfx_initted;
    }

    /* Init graphics pipeline */ {
        isize vert_code_size;
        auto vert_code = SDL_LoadFile("gfx.vert.spv", (size_t *)&vert_code_size);
        defer(SDL_free(vert_code));

        SDL_GPUShaderCreateInfo vert_info = {};
        vert_info.code_size    = vert_code_size;
        vert_info.code         = (u8 *)vert_code;
        vert_info.entrypoint   = "main";
        vert_info.format       = SDL_GPU_SHADERFORMAT_SPIRV;
        vert_info.stage        = SDL_GPU_SHADERSTAGE_VERTEX;
        vert_info.num_samplers = 0;
        vert_info.num_storage_textures = 0;
        vert_info.num_storage_buffers  = 0;
        vert_info.num_uniform_buffers  = 0;
        auto vert_shader = SDL_CreateGPUShader(gfx_device, &vert_info);
        defer(SDL_ReleaseGPUShader(gfx_device, vert_shader));

        isize frag_code_size;
        auto frag_code = SDL_LoadFile("gfx.frag.spv", (size_t *)&frag_code_size);
        defer(SDL_free(frag_code));

        SDL_GPUShaderCreateInfo frag_info = {};
        frag_info.code_size    = frag_code_size;
        frag_info.code         = (u8 *)frag_code;
        frag_info.entrypoint   = "main";
        frag_info.format       = SDL_GPU_SHADERFORMAT_SPIRV;
        frag_info.stage        = SDL_GPU_SHADERSTAGE_FRAGMENT;
        frag_info.num_samplers = 0;
        frag_info.num_storage_textures = 0;
        frag_info.num_storage_buffers  = 0;
        frag_info.num_uniform_buffers  = 0;
        auto frag_shader = SDL_CreateGPUShader(gfx_device, &frag_info);
        defer(SDL_ReleaseGPUShader(gfx_device, frag_shader));

        SDL_GPUColorTargetDescription color_desc = {};
        color_desc.format = SDL_GetGPUSwapchainTextureFormat(gfx_device, gfx_window);
        
        SDL_GPUGraphicsPipelineTargetInfo target_info = {};
        target_info.color_target_descriptions = &color_desc;
        target_info.num_color_targets = 1;

        SDL_GPUGraphicsPipelineCreateInfo pipe_info = {};
        pipe_info.vertex_shader   = vert_shader;
        pipe_info.fragment_shader = frag_shader;
        pipe_info.primitive_type  = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipe_info.target_info     = target_info;
        gfx_pipeline = SDL_CreateGPUGraphicsPipeline(gfx_device, &pipe_info);
    }

    gfx_initted = true;
    return gfx_initted;
}

IDEF void gfx_quit() {
    if (gfx_initted) {
        SDL_ReleaseGPUGraphicsPipeline(gfx_device, gfx_pipeline);
        SDL_ReleaseWindowFromGPUDevice(gfx_device, gfx_window);
        SDL_DestroyGPUDevice(gfx_device);
    }
}

IDEF void gfx_draw() {
    auto command_buf = SDL_AcquireGPUCommandBuffer(gfx_device);
    SDL_GPUTexture *swapchain_tex;
    ASSERT(SDL_WaitAndAcquireGPUSwapchainTexture(command_buf, gfx_window, &swapchain_tex, NULL, NULL));
    defer(SDL_SubmitGPUCommandBuffer(command_buf));

    if (swapchain_tex != NULL) {
        SDL_FColor clear_color = {0.0f, 0.2f, 0.4f, 1.0f};
        SDL_GPUColorTargetInfo color_info = {};
        color_info.clear_color = clear_color;
        color_info.load_op = SDL_GPU_LOADOP_CLEAR;
        color_info.store_op = SDL_GPU_STOREOP_STORE;
        color_info.texture = swapchain_tex;

        auto render_pass = SDL_BeginGPURenderPass(command_buf, &color_info, 1, NULL);
        defer(SDL_EndGPURenderPass(render_pass));
        SDL_BindGPUGraphicsPipeline(render_pass, gfx_pipeline);
        SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
    }
}

#endif // GFX_IMPLEMENTATION

#endif // GFX_H
