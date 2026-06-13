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

    usize code_size;
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

    return SDL_CreateGPUShader(gfx->device, &info);
}

static void gfx_init_quad_batch(void) {
    auto *batch = &gfx->quad_batch;

    /* Create pipeline */ {
        auto *vert_shader = gfx_load_shader(LIT("shader/quad.vert.1u.spv"));
        defer (SDL_ReleaseGPUShader(gfx->device, vert_shader));

        auto *frag_shader = gfx_load_shader(LIT("shader/quad.frag.1s.spv"));
        defer (SDL_ReleaseGPUShader(gfx->device, frag_shader));

        const i32 NUM_VERTEX_ATTRIBUTES = 3;
        SDL_GPUVertexAttribute vert_attributes[NUM_VERTEX_ATTRIBUTES] = {};
        vert_attributes[0].location    = 0;
        vert_attributes[0].buffer_slot = 0;
        vert_attributes[0].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        vert_attributes[0].offset      = offsetof(Gfx_Quad_Vertex, position);
        vert_attributes[1].location    = 1;
        vert_attributes[1].buffer_slot = 0;
        vert_attributes[1].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        vert_attributes[1].offset      = offsetof(Gfx_Quad_Vertex, texcoord);
        vert_attributes[2].location    = 2;
        vert_attributes[2].buffer_slot = 0;
        vert_attributes[2].format      = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
        vert_attributes[2].offset      = offsetof(Gfx_Quad_Vertex, color);

        const i32 NUM_VERTEX_BUFFERS = 1;
        SDL_GPUVertexBufferDescription vert_descriptions[NUM_VERTEX_BUFFERS] = {};
        vert_descriptions[0].slot  = 0;
        vert_descriptions[0].pitch = sizeof(Gfx_Quad_Vertex);
        vert_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        const i32 NUM_TARGET_COLORS = 1;
        SDL_GPUColorTargetDescription target_colors[NUM_TARGET_COLORS] = {};
        target_colors[0].format = SDL_GetGPUSwapchainTextureFormat(gfx->device, gfx->window);
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

        batch->pipeline = SDL_CreateGPUGraphicsPipeline(gfx->device, &pipe_info);
    }

    /* Create buffers */ {
        u32 offset = 0;

        SDL_GPUBufferCreateInfo vertex_info = {};
        vertex_info.size  = GFX_NUM_VERTICES_PER_QUAD * GFX_MAX_QUADS_PER_BATCH * sizeof(Gfx_Quad_Vertex);
        vertex_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        batch->vertex_region.buffer = SDL_CreateGPUBuffer(gfx->device, &vertex_info);
        batch->vertex_region.size   = vertex_info.size;
        batch->vertex_region.offset = offset;

        offset += vertex_info.size;

        SDL_GPUBufferCreateInfo index_info = {};
        index_info.size  = GFX_NUM_INDICES_PER_QUAD * GFX_MAX_QUADS_PER_BATCH * sizeof(Gfx_Index);
        index_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        batch->index_region.buffer = SDL_CreateGPUBuffer(gfx->device, &index_info);
        batch->index_region.size   = index_info.size;
        batch->index_region.offset = offset;

        offset += index_info.size;

        SDL_GPUTransferBufferCreateInfo upload_info = {};
        upload_info.size  = offset;
        upload_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        batch->upload_buffer = SDL_CreateGPUTransferBuffer(gfx->device, &upload_info);
    }

    /* Create white texture */ {
        u32 white = 0xffffffff; // 1x1 RGBA

        SDL_GPUTransferBufferCreateInfo upload_info = {};
        upload_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        upload_info.size  = sizeof(white);

        auto *upload_buf = SDL_CreateGPUTransferBuffer(gfx->device, &upload_info);
        defer (SDL_ReleaseGPUTransferBuffer(gfx->device, upload_buf));

        auto *upload_ptr = (u32 *)SDL_MapGPUTransferBuffer(gfx->device, upload_buf, false);
        mem_copy(upload_ptr, &white, sizeof(white));
        SDL_UnmapGPUTransferBuffer(gfx->device, upload_buf);

        SDL_GPUTextureCreateInfo tex_info = {};
        tex_info.type       = SDL_GPU_TEXTURETYPE_2D;
        tex_info.format     = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        tex_info.width      = 1;
        tex_info.height     = 1;
        tex_info.num_levels = 1;
        tex_info.usage      = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        tex_info.layer_count_or_depth = 1;

        batch->white_texture = SDL_CreateGPUTexture(gfx->device, &tex_info);

        SDL_GPUSamplerCreateInfo sampler_info = {};
        sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
        sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;

        batch->white_sampler = SDL_CreateGPUSampler(gfx->device, &sampler_info);

        auto *command_buf = SDL_AcquireGPUCommandBuffer(gfx->device);
        defer (SDL_SubmitGPUCommandBuffer(command_buf));

        auto *copy_pass = SDL_BeginGPUCopyPass(command_buf);
        defer (SDL_EndGPUCopyPass(copy_pass));

        SDL_GPUTextureTransferInfo transfer_info = {};
        transfer_info.transfer_buffer = upload_buf;
        transfer_info.offset = 0;

        SDL_GPUTextureRegion transfer_region = {};
        transfer_region.texture = batch->white_texture;
        transfer_region.w = 1;
        transfer_region.h = 1;
        transfer_region.d = 1;

        SDL_UploadToGPUTexture(copy_pass, &transfer_info, &transfer_region, false);
    }
}

static void gfx_free_quad_batch() {
    auto *batch = &gfx->quad_batch;

    SDL_ReleaseGPUSampler(gfx->device, batch->white_sampler);
    SDL_ReleaseGPUTexture(gfx->device, batch->white_texture);
    SDL_ReleaseGPUTransferBuffer(gfx->device, batch->upload_buffer);
    SDL_ReleaseGPUBuffer(gfx->device, batch->index_region.buffer);
    SDL_ReleaseGPUBuffer(gfx->device, batch->vertex_region.buffer);
    SDL_ReleaseGPUGraphicsPipeline(gfx->device, batch->pipeline);

    mem_zero_ptr(batch);
}

static void gfx_render_quad_batch(void) {
    auto *command_buf = SDL_AcquireGPUCommandBuffer(gfx->device);
    defer (SDL_SubmitGPUCommandBuffer(command_buf));

    SDL_GPUTexture *swapchain_tex;
    u32 swapchain_width, swapchain_height;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buf, gfx->window, &swapchain_tex, &swapchain_width, &swapchain_height);
    if (!swapchain_tex) return;

    Mat4 proj = mat4_ortho(0.0f, swapchain_width, swapchain_height, 0.0f, -1.0f, 1.0f);

    /* Upload */ {
        auto *batch = &gfx->quad_batch;

        SDL_PushGPUVertexUniformData(command_buf, 0, &proj, sizeof(Mat4));

        auto *copy_pass = SDL_BeginGPUCopyPass(command_buf);
        defer (SDL_EndGPUCopyPass(copy_pass));

        {
            SDL_GPUTransferBufferLocation copy_src = {};
            copy_src.transfer_buffer = batch->upload_buffer;
            copy_src.offset = batch->vertex_region.offset;

            SDL_GPUBufferRegion copy_dst = {};
            copy_dst.buffer = batch->vertex_region.buffer;
            copy_dst.size   = batch->vertex_region.size;

            SDL_UploadToGPUBuffer(copy_pass, &copy_src, &copy_dst, false);
        }

        {
            SDL_GPUTransferBufferLocation copy_src = {};
            copy_src.transfer_buffer = batch->upload_buffer;
            copy_src.offset = batch->index_region.offset;

            SDL_GPUBufferRegion copy_dst = {};
            copy_dst.buffer = batch->index_region.buffer;
            copy_dst.size   = batch->index_region.size;

            SDL_UploadToGPUBuffer(copy_pass, &copy_src, &copy_dst, false);
        }
    }

    SDL_FColor fcolor = {};
    fcolor.r = gfx->clear_color.r / 255;
    fcolor.g = gfx->clear_color.g / 255;
    fcolor.b = gfx->clear_color.b / 255;
    fcolor.a = gfx->clear_color.a / 255;

    SDL_GPUColorTargetInfo color_info = {};
    color_info.clear_color = fcolor;
    color_info.load_op     = SDL_GPU_LOADOP_CLEAR;
    color_info.store_op    = SDL_GPU_STOREOP_STORE;
    color_info.texture     = swapchain_tex;

    /* Render */ {
        auto *batch = &gfx->quad_batch;

        auto *render_pass = SDL_BeginGPURenderPass(command_buf, &color_info, 1, NULL);
        defer (SDL_EndGPURenderPass(render_pass));

        SDL_BindGPUGraphicsPipeline(render_pass, batch->pipeline);

        {
            const i32 num_bindings = 1;
            SDL_GPUBufferBinding bindings[num_bindings] = {};

            bindings[0].buffer = batch->vertex_region.buffer;
            bindings[0].offset = 0;

            SDL_BindGPUVertexBuffers(render_pass, 0, bindings, num_bindings);
        }

        {
            const i32 num_bindings = 1;
            SDL_GPUBufferBinding bindings[num_bindings] = {};

            bindings[0].buffer = batch->index_region.buffer;
            bindings[0].offset = 0;

            SDL_BindGPUIndexBuffer(render_pass, bindings, GFX_INDEX_ELEMENT_SIZE);
        }

        {
            const i32 num_bindings = 1;
            SDL_GPUTextureSamplerBinding bindings[num_bindings] = {};

            bindings[0].texture = batch->white_texture;
            bindings[0].sampler = batch->white_sampler;

            SDL_BindGPUFragmentSamplers(render_pass, 0, bindings, num_bindings);
        }

        u32 num_current_indices = GFX_NUM_INDICES_PER_QUAD * batch->push_count;
        u32 num_draw_indices    = CLAMP(num_current_indices, 0, GFX_MAX_QUAD_INDICES_PER_BATCH);
        SDL_DrawGPUIndexedPrimitives(render_pass, num_draw_indices, 1, 0, 0, 0);
    }
}

static void gfx_flush_quad_batch(void) {
    auto *batch = &gfx->quad_batch;

    ASSERT(0 < batch->push_count && batch->push_count <= GFX_MAX_QUADS_PER_BATCH);

    gfx_render_quad_batch();

    batch->push_count = 0;
}

void gfx_init(const Allocator &a, SDL_Window *window) {
    gfx = alloc_struct(a, Gfx_State);

    mem_zero_ptr(gfx);

    gfx->allocator = a;
    gfx->window = window;
    gfx->device = SDL_CreateGPUDevice(GFX_SHADER_FORMAT, true, NULL);

    ASSERT(SDL_ClaimWindowForGPUDevice(gfx->device, gfx->window));

    #if 1
    SDL_SetGPUSwapchainParameters(gfx->device, gfx->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
    #endif

    gfx_init_quad_batch();
}

void gfx_quit(void) {
    gfx_free_quad_batch();

    SDL_ReleaseWindowFromGPUDevice(gfx->device, gfx->window);
    SDL_DestroyGPUDevice(gfx->device);

    allocator_free(gfx->allocator, gfx);
}

void gfx_set_clear_color(Gfx_Color color) {
    gfx->clear_color = color;
}

void gfx_begin_drawing(void) {
    ASSERT(!gfx->drawing);

    gfx->drawing = true;
    gfx->quad_batch.upload_ptr = (u8 *)SDL_MapGPUTransferBuffer(gfx->device, gfx->quad_batch.upload_buffer, false);
}

void gfx_end_drawing(void) {
    ASSERT(gfx->drawing);

    gfx_flush_quad_batch();

    SDL_UnmapGPUTransferBuffer(gfx->device, gfx->quad_batch.upload_buffer);
    gfx->drawing = false;
}

void gfx_draw_rectangle(f32 x, f32 y, f32 w, f32 h, Gfx_Color color) {
    auto *batch = &gfx->quad_batch;

    if (batch->push_count >= GFX_MAX_QUADS_PER_BATCH) {
        gfx_flush_quad_batch();
    }

    u32 vertex_count = GFX_NUM_VERTICES_PER_QUAD * batch->push_count;
    u32 index_count  = GFX_NUM_INDICES_PER_QUAD  * batch->push_count;
    u16 index_base   = (u16)vertex_count;

    auto *vertices = (Gfx_Quad_Vertex *)(batch->upload_ptr + batch->vertex_region.offset);

    vertices[vertex_count++] = {
        vec2_make(x, y),        // position
        vec2_make(0.0f, 1.0f),  // texcoord
        color,                  // color
    };

    vertices[vertex_count++] = {
        vec2_make(x + w, y),    // position
        vec2_make(1.0f, 1.0f),  // texcoord
        color,                  // color
    };

    vertices[vertex_count++] = {
        vec2_make(x + w, y + h), // position
        vec2_make(1.0f, 0.0f),   // texcoord
        color,                   // color
    };

    vertices[vertex_count++] = {
        vec2_make(x, y + h),     // position
        vec2_make(0.0f, 0.0f),   // texcoord
        color,                   // color
    };

    auto *indices = (Gfx_Index *)(batch->upload_ptr + batch->index_region.offset);

    indices[index_count++] = index_base + 0;
    indices[index_count++] = index_base + 1;
    indices[index_count++] = index_base + 2;

    indices[index_count++] = index_base + 2;
    indices[index_count++] = index_base + 3;
    indices[index_count++] = index_base + 0;

    batch->push_count += 1;
}
