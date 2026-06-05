#include "gfx.h"
#include "base.h"
#include "linalg.h"

#include <SDL3/SDL.h>

//
// Load shader from a file with this name format: <name>.<stage>.<optionals>.<extension>
//
// Examples:
//   example.vert.spv
//   example.vert.1s.2t.0b.2u.spv
//   example.frag.spv
//
// I think putting hints on the file name is simpler and more practical than
// using a sidecar file to store shader metadata. Those hints are:
//   - Shader format, indicated by file extension
//   - Shader stage, indicated by "vert" or "frag"
//   - Samplers, e.g. "1s"
//   - Storage textures, e.g. "2t"
//   - Storage buffers, e.g. "0b"
//   - Uniform buffers, e.g. "1u"
//
static SDL_GPUShader *gfx_load_shader(const String &file) {
    auto s = arena_begin_scratch(NULL, 0);
    defer (arena_end_scratch(s));

    Array<String> parts = string_split(arena_allocator(s.arena), file, LIT("."));
    if (parts.len < 3) return NULL;

    String parsed_name      = parts[0];
    String parsed_stage     = parts[1];
    String parsed_extension = parts[parts.len - 1];

    SDL_GPUShaderFormat format;
    const char *entry;
         if (parsed_extension == "spv")  { format = SDL_GPU_SHADERFORMAT_SPIRV; entry = "main";  }
    else if (parsed_extension == "dxil") { format = SDL_GPU_SHADERFORMAT_DXIL;  entry = "main";  }
    else if (parsed_extension == "msl")  { format = SDL_GPU_SHADERFORMAT_MSL;   entry = "main0"; }
    else return NULL;

    SDL_GPUShaderStage stage;
         if (parsed_stage == "vert") stage = SDL_GPU_SHADERSTAGE_VERTEX;
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
    void *code = SDL_LoadFile(string_to_cstr(arena_allocator(s.arena), file), (size_t *)&code_size);
    if (!code) return NULL;
    defer (SDL_free(code));

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

    return SDL_CreateGPUShader(gfx_state->device, &info);
}

void gfx_init(const Allocator &a, SDL_Window *window) {
    gfx_state = alloc_struct(a, Gfx_State);

    mem_zero_ptr(gfx_state);

    gfx_state->window = window;
    gfx_state->device = SDL_CreateGPUDevice(GFX_SHADER_FORMAT, true, NULL);

    ASSERT(SDL_ClaimWindowForGPUDevice(gfx_state->device, gfx_state->window));

    auto vert_shader = gfx_load_shader(LIT("shader/gfx.vert.spv"));
    defer (SDL_ReleaseGPUShader(gfx_state->device, vert_shader));

    auto frag_shader = gfx_load_shader(LIT("shader/gfx.frag.spv"));
    defer (SDL_ReleaseGPUShader(gfx_state->device, frag_shader));

    /* Create pipeline */ {
        const i32 NUM_VERTEX_ATTRIBUTES = 3;
        SDL_GPUVertexAttribute vert_attributes[NUM_VERTEX_ATTRIBUTES];
        mem_zero_array(vert_attributes);
        vert_attributes[0].location    = 0;
        vert_attributes[0].buffer_slot = 0;
        vert_attributes[0].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        vert_attributes[0].offset      = offsetof(Gfx_Vertex2D, position);
        vert_attributes[1].location    = 1;
        vert_attributes[1].buffer_slot = 0;
        vert_attributes[1].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        vert_attributes[1].offset      = offsetof(Gfx_Vertex2D, texcoord);
        vert_attributes[2].location    = 2;
        vert_attributes[2].buffer_slot = 0;
        vert_attributes[2].format      = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
        vert_attributes[2].offset      = offsetof(Gfx_Vertex2D, color);

        const i32 NUM_VERTEX_BUFFERS = 1;
        SDL_GPUVertexBufferDescription vert_descriptions[NUM_VERTEX_BUFFERS];
        mem_zero_array(vert_descriptions);
        vert_descriptions[0].slot  = 0;
        vert_descriptions[0].pitch = sizeof(Gfx_Vertex2D);
        vert_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        const i32 NUM_TARGET_COLORS = 1;
        SDL_GPUColorTargetDescription target_colors[NUM_TARGET_COLORS];
        mem_zero_array(target_colors);
        target_colors[0].format = SDL_GetGPUSwapchainTextureFormat(gfx_state->device, gfx_state->window);
        target_colors[0].blend_state.enable_blend          = true;
        target_colors[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        target_colors[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        target_colors[0].blend_state.color_blend_op        = SDL_GPU_BLENDOP_ADD;
        target_colors[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        target_colors[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        target_colors[0].blend_state.alpha_blend_op        = SDL_GPU_BLENDOP_ADD;

        SDL_GPUGraphicsPipelineCreateInfo pipe_info = {};
        pipe_info.vertex_shader                                 = vert_shader;
        pipe_info.fragment_shader                               = frag_shader;
        pipe_info.vertex_input_state.vertex_attributes          = vert_attributes;
        pipe_info.vertex_input_state.num_vertex_attributes      = NUM_VERTEX_ATTRIBUTES;
        pipe_info.vertex_input_state.vertex_buffer_descriptions = vert_descriptions;
        pipe_info.vertex_input_state.num_vertex_buffers         = NUM_VERTEX_BUFFERS;
        pipe_info.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipe_info.rasterizer_state.fill_mode                    = SDL_GPU_FILLMODE_FILL;
        pipe_info.rasterizer_state.cull_mode                    = SDL_GPU_CULLMODE_NONE;
        pipe_info.depth_stencil_state.enable_depth_test         = false;
        pipe_info.depth_stencil_state.enable_depth_write        = false;
        pipe_info.target_info.color_target_descriptions         = target_colors;
        pipe_info.target_info.num_color_targets                 = NUM_TARGET_COLORS;

        gfx_state->_2d.pipeline = SDL_CreateGPUGraphicsPipeline(gfx_state->device, &pipe_info);
    }

    /* Create buffers */ {
        const Gfx_Vertex2D vertices[] = {
            { vec2_make(-0.5f, -0.5f), vec2_make(0.0f, 1.0f), GFX_WHITE }, // Bottom-left
            { vec2_make( 0.5f, -0.5f), vec2_make(1.0f, 1.0f), GFX_WHITE }, // Bottom-right
            { vec2_make( 0.5f,  0.5f), vec2_make(1.0f, 0.0f), GFX_WHITE }, // Top-right
            { vec2_make(-0.5f,  0.5f), vec2_make(0.0f, 0.0f), GFX_WHITE }, // Top-left
        };

        const u16 indices[] = {
            0, 1, 2,
            2, 3, 0,
        };

        SDL_GPUBufferCreateInfo vertex_info = {};
        vertex_info.size  = sizeof(vertices);
        vertex_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        gfx_state->_2d.vertex_buffer = SDL_CreateGPUBuffer(gfx_state->device, &vertex_info);

        SDL_GPUBufferCreateInfo index_info = {};
        index_info.size  = sizeof(indices);
        index_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        gfx_state->_2d.index_buffer = SDL_CreateGPUBuffer(gfx_state->device, &index_info);

        SDL_GPUTransferBufferCreateInfo transfer_info = {};
        transfer_info.size   = vertex_info.size + index_info.size;
        transfer_info.usage  = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        auto transfer_buffer = SDL_CreateGPUTransferBuffer(gfx_state->device, &transfer_info);
        defer (SDL_ReleaseGPUTransferBuffer(gfx_state->device, transfer_buffer));

        auto transfer_map = (u8 *)SDL_MapGPUTransferBuffer(gfx_state->device, transfer_buffer, false);
        isize transfer_offset = 0;
        mem_copy(transfer_map + transfer_offset, vertices, vertex_info.size);
        transfer_offset += vertex_info.size;
        mem_copy(transfer_map + transfer_offset, indices, index_info.size);
        transfer_offset += index_info.size;
        SDL_UnmapGPUTransferBuffer(gfx_state->device, transfer_buffer);

        auto command_buf = SDL_AcquireGPUCommandBuffer(gfx_state->device);
        defer (SDL_SubmitGPUCommandBuffer(command_buf));

        auto copy_pass = SDL_BeginGPUCopyPass(command_buf);
        defer (SDL_EndGPUCopyPass(copy_pass));

        isize upload_offset = 0;

        /* Vertex buffer upload */ {
            SDL_GPUTransferBufferLocation copy_src = {};
            copy_src.transfer_buffer = transfer_buffer;
            copy_src.offset = upload_offset;

            SDL_GPUBufferRegion copy_dst = {};
            copy_dst.buffer = gfx_state->_2d.vertex_buffer;
            copy_dst.offset = 0;
            copy_dst.size   = vertex_info.size;

            SDL_UploadToGPUBuffer(copy_pass, &copy_src, &copy_dst, false);

            upload_offset += copy_dst.size;
        }


        /* Index buffer upload */ {
            SDL_GPUTransferBufferLocation copy_src = {};
            copy_src.transfer_buffer = transfer_buffer;
            copy_src.offset = upload_offset;

            SDL_GPUBufferRegion copy_dst = {};
            copy_dst.buffer = gfx_state->_2d.index_buffer;
            copy_dst.offset = 0;
            copy_dst.size   = index_info.size;

            SDL_UploadToGPUBuffer(copy_pass, &copy_src, &copy_dst, false);

            upload_offset += copy_dst.size;
        }
    }
}

void gfx_quit() {
    SDL_ReleaseGPUBuffer(gfx_state->device, gfx_state->_2d.index_buffer);
    SDL_ReleaseGPUBuffer(gfx_state->device, gfx_state->_2d.vertex_buffer);
    SDL_ReleaseGPUGraphicsPipeline(gfx_state->device, gfx_state->_2d.pipeline);
    SDL_ReleaseWindowFromGPUDevice(gfx_state->device, gfx_state->window);
    SDL_DestroyGPUDevice(gfx_state->device);

    allocator_free(gfx_state->allocator, gfx_state);
}

void gfx_draw(Gfx_Color clear_color) {
    auto command_buf = SDL_AcquireGPUCommandBuffer(gfx_state->device);
    defer (SDL_SubmitGPUCommandBuffer(command_buf));

    SDL_GPUTexture *swapchain_tex;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buf, gfx_state->window, &swapchain_tex, NULL, NULL);

    if (!swapchain_tex) return;

    SDL_FColor fcolor = {};
    fcolor.r = clear_color.r / 255;
    fcolor.g = clear_color.g / 255;
    fcolor.b = clear_color.b / 255;
    fcolor.a = clear_color.a / 255;

    SDL_GPUColorTargetInfo color_info = {};
    color_info.clear_color = fcolor;
    color_info.load_op     = SDL_GPU_LOADOP_CLEAR;
    color_info.store_op    = SDL_GPU_STOREOP_STORE;
    color_info.texture     = swapchain_tex;

    auto render_pass = SDL_BeginGPURenderPass(command_buf, &color_info, 1, NULL);
    defer (SDL_EndGPURenderPass(render_pass));

    SDL_BindGPUGraphicsPipeline(render_pass, gfx_state->_2d.pipeline);

    SDL_GPUBufferBinding vertex_binding = {};
    vertex_binding.buffer = gfx_state->_2d.vertex_buffer;
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);

    SDL_GPUBufferBinding index_binding = {};
    index_binding.buffer = gfx_state->_2d.index_buffer;
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(render_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
}
