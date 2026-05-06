#ifndef GFX_H
#define GFX_H

//
// DECLARATION
//

#include "base.h"

#pragma push_macro("internal")
#undef internal
#include <SDL3/SDL.h>
#pragma pop_macro("internal")

#ifdef GFX_IMPLEMENTATION
    #define GFX_DEF
#else
    #define GFX_DEF extern
#endif

extern bool gfx_initted;
extern SDL_Window *gfx_window;
extern SDL_GPUDevice *gfx_device;
extern SDL_GPUGraphicsPipeline *gfx_pipeline;

#define GFX_SHADER_FORMAT (SDL_GPU_SHADERFORMAT_SPIRV)

GFX_DEF SDL_GPUShader *gfx_load_shader(const String &file);

GFX_DEF bool gfx_init(SDL_Window *window);
GFX_DEF void gfx_quit();
GFX_DEF void gfx_draw();

//
// IMPLEMENTATION
//

#ifdef GFX_IMPLEMENTATION

bool gfx_initted = false;
SDL_Window *gfx_window = NULL;
SDL_GPUDevice *gfx_device = NULL;
SDL_GPUGraphicsPipeline *gfx_pipeline = NULL;

//
// Load shader from a file with this name format: <name>.<stage>.<optionals>.<extension>
//
// Examples:
//     example.vert.spv
//     example.vert.1s.2t.0b.2u.spv
//     example.frag.spv
//
// I think putting hints on the file name is simpler and more practical than
// using a sidecar file to store shader metadata. Those hints are:
//     - Shader format, indicated by file extension
//     - Shader stage, indicated by "vert" or "frag"
//     - Samplers, e.g. "1s"
//     - Storage textures, e.g. "2t"
//     - Storage buffers, e.g. "0b"
//     - Uniform buffers, e.g. "1u"
//
GFX_DEF SDL_GPUShader *gfx_load_shader(const String &file) {
    auto s = acquire_scratch_arena();
    defer(release_scratch_arena(s));

    Array<String> parts = string_split(s.arena, file, LIT("."));
    if (parts.len < 3) return NULL;

    String parsed_name      = parts[0];
    String parsed_stage     = parts[1];
    String parsed_extension = parts[parts.len - 1];

    SDL_GPUShaderFormat format;
    const char *entry;
    if      (parsed_extension == "spv")  { format = SDL_GPU_SHADERFORMAT_SPIRV; entry = "main";  }
    else if (parsed_extension == "dxil") { format = SDL_GPU_SHADERFORMAT_DXIL;  entry = "main";  }
    else if (parsed_extension == "msl")  { format = SDL_GPU_SHADERFORMAT_MSL;   entry = "main0"; }
    else return NULL;

    SDL_GPUShaderStage stage;
    if      (parsed_stage == "vert") stage = SDL_GPU_SHADERSTAGE_VERTEX;
    else if (parsed_stage == "frag") stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    else return NULL;

    i32 num_samplers    = 0;
    i32 num_storage_tex = 0;
    i32 num_storage_buf = 0;
    i32 num_uniform_buf = 0;
    for (isize i = 2; i < parts.len - 1; i++) {
        String hint = parts[i];

        String parsed_num = string_empty();
        char parsed_char = 0;
        for (isize j = 0; j < hint.len; j++) {
            if (byte_is_digit(hint[j])) {
                parsed_num = string_make(hint.data, j + 1);
            } else {
                parsed_char = hint[j];
                break;
            }
        }

        i32 num = 0;
        for (isize j = 0; j < parsed_num.len; j++) {
            num *= 10;
            num += parsed_num[j] - '0';
        }
        switch (parsed_char) {
            case 's': num_samplers    = num; break;
            case 't': num_storage_tex = num; break;
            case 'b': num_storage_buf = num; break;
            case 'u': num_uniform_buf = num; break;
        }
    }

    isize code_size;
    void *code = SDL_LoadFile(string_to_cstr(s.arena, file), (size_t *)&code_size);
    if (!code) return NULL;
    defer(SDL_free(code));

    SDL_GPUShaderCreateInfo info = {};
    info.code_size  = code_size;
    info.code       = (u8 *)code;
    info.entrypoint = entry;
    info.format     = format;
    info.stage      = stage;
    info.num_samplers         = num_samplers;
    info.num_storage_textures = num_storage_tex;
    info.num_storage_buffers  = num_storage_buf;
    info.num_uniform_buffers  = num_uniform_buf;

    return SDL_CreateGPUShader(gfx_device, &info);
}

GFX_DEF bool gfx_init(SDL_Window *window) {
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
        auto vert_shader = gfx_load_shader(LIT("shader/gfx.vert.1u.spv"));
        defer(SDL_ReleaseGPUShader(gfx_device, vert_shader));

        auto frag_shader = gfx_load_shader(LIT("shader/gfx.frag.spv"));
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

GFX_DEF void gfx_quit() {
    if (gfx_initted) {
        SDL_ReleaseGPUGraphicsPipeline(gfx_device, gfx_pipeline);
        SDL_ReleaseWindowFromGPUDevice(gfx_device, gfx_window);
        SDL_DestroyGPUDevice(gfx_device);
    }
    gfx_initted = false;
}

GFX_DEF void gfx_draw() {
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
