//
//  main.c
//  GpuByExample
//
//  Created by Jonathan Fischer on 3/6/25.
//

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// I'm trying to keep the interesting / relevant parts of each chapter
// in main.c, and moving stuff we've already seen out to other files.
#include "AppContext.h"

// This time through we're actually drawing something, so we need _geometry_. And for
// that, we need to specify exactly what we're going to give the GPU. For now, that's
// a position and color.
//
// NOTE: In theory we could use both a 2 component position (only x and y) and a
// component color (red, green, and blue) and provide the missing pieces in the
// shader, but for Metal at least there are requirements about certain pieces of
// data being aligned to 16-bytes. It's easier to just use the full four components
// for both, at least for now.
typedef struct GBEVector4 {
    float x, y, z, w;
} GBEVector4;

typedef struct GBEColor {
    float r, g, b, a;
} GBEColor;

typedef struct GBEVertex {
    GBEVector4 position;
    GBEColor color;
} GBEVertex;

// Now we need the actual triangle geometry. I'm cheating a bit here and choosing to
// draw a triangle that's already defined in clip space, so we don't need to worry
// about any math yet. (Or more specifically, providing a transformation to the vertex
// shader.)
static const size_t kNumVertices = 3;

static const GBEVertex kVertices[] = {
    { .position = {  0.0,  0.5, 0, 1 }, .color = { 1.0, 0.0, 0.0, 1.0 } },
    { .position = { -0.5, -0.5, 0, 1 }, .color = { 0.0, 1.0, 0.0, 1.0 } },
    { .position = {  0.5, -0.5, 0, 1 }, .color = { 0.0, 0.0, 1.0, 1.0 } },
};

// Loading shaders is a little involved, so I've moved that out to a standalone function.
static SDL_AppResult LoadShaders(SDL_Storage *storage, SDL_GPUDevice* device, SDL_GPUShader** vertexShader, SDL_GPUShader** fragmentShader)
{
    void* code = NULL;
    Uint64 codeSize = 0L;
    if (!SDL_GetStorageFileSize(storage, "SingleTriangle.msl", &codeSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to determine size of file 'SingleTriangle.msl': %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Allocate memory to read the file into, then do that.
    code = SDL_malloc(codeSize);
    if (!SDL_ReadStorageFile(storage, "SingleTriangle.msl", code, codeSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to read file 'SingleTriangle.msl': %s", SDL_GetError());
        SDL_free(code);
        return SDL_APP_FAILURE;
    }

    // When creating a shader, we need to tell it what stage of the pipeline it's meant
    // to process, how many inputs it takes, and what the entry function's name is. This
    // one covers the vertex shader
    SDL_GPUShaderCreateInfo createInfo = {
        .code = code,
        .code_size = codeSize,
        .entrypoint = "vertex_main",
        .format = SDL_GPU_SHADERFORMAT_MSL,
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .num_samplers = 0,
        .num_uniform_buffers = 0,
        .num_storage_buffers = 1,
        .num_storage_textures = 0
    };

    SDL_GPUShader* vs = SDL_CreateGPUShader(device, &createInfo);
    if (vs == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create vertex shader: %s", SDL_GetError());
        SDL_free(code);
        return SDL_APP_FAILURE;
    }

    // And this one the fragment shader.
    createInfo.entrypoint = "fragment_main";
    createInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    createInfo.num_storage_buffers = 0;
    SDL_GPUShader* fs = SDL_CreateGPUShader(device, &createInfo);
    if (fs == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create fragment shader: %s", SDL_GetError());
        SDL_ReleaseGPUShader(device, vs);
        SDL_free(code);
        return SDL_APP_FAILURE;
    }

    // And we're done, let's cleanup and return
    SDL_free(code);
    code = NULL;

    *vertexShader = vs;
    *fragmentShader = fs;
    return SDL_APP_CONTINUE;
}

static SDL_AppResult BuildPipeline(AppContext* context)
{
    SDL_GPUShader* vertexShader;
    SDL_GPUShader* fragmentShader;

    SDL_AppResult rc = LoadShaders(context->titleStorage, context->device, &vertexShader, &fragmentShader);
    if (rc != SDL_APP_CONTINUE) {
        return rc;
    }

    // Lots of stuff here! Let's take it one chunk at a time. First, we describe the
    // render target, which includes both a target pixel format and how we want to
    // blend colors into it.
    SDL_GPUTextureFormat targetFormat = SDL_GetGPUSwapchainTextureFormat(context->device, context->window);

    SDL_GPUGraphicsPipelineTargetInfo targetInfo = {
        .num_color_targets = 1,
        .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
            .format = targetFormat,
            .blend_state = {
                .enable_blend = true,
                .color_blend_op = SDL_GPU_BLENDOP_ADD,
                .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
            }
        }}
    };

    // Then, how we want our primitives rasterized.
    SDL_GPURasterizerState rasterizerState = {
        // We want to draw the interiors of the primitives, not just outlines
        .fill_mode = SDL_GPU_FILLMODE_FILL,

        // Don't draw primitives that are facing away from the screen
        .cull_mode = SDL_GPU_CULLMODE_BACK,

        // Our vertices are provided in counter-clockwise order
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
    };

    // That should be enough to create a pipeline! We provide the target info and
    // rasterizer state defined above, we want to draw triangles, and we want to
    // do it using the vertex and fragment shaders we loaded.
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .target_info = targetInfo,
        .rasterizer_state = rasterizerState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader
    };

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(context->device, &pipelineCreateInfo);
    if (pipeline == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create graphics pipeline: %s", SDL_GetError());
    }

    // Now that the pipeline has been created, it's holding on to references to the shaders, so we
    // don't need to keep them around anymore. (I think they're reference counted, and the pipeline
    // retained them.)
    SDL_ReleaseGPUShader(context->device, vertexShader);
    SDL_ReleaseGPUShader(context->device, fragmentShader);

    // Store the created pipeline (or the NULL if it failed) in our application context and be done.
    context->pipeline = pipeline;
    return pipeline != NULL ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
}

static SDL_AppResult BuildVertexBuffer(AppContext* context)
{
    // Creating a vertex buffer is easy: all we really need to tell it is what
    // we're going to use it for and how much space to allocate.
    SDL_GPUBufferCreateInfo bufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
        .size = sizeof(GBEVertex) * kNumVertices
    };
    SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(context->device, &bufferCreateInfo);

    // Getting data _into_ the vertex buffer is way more work. We can't just copy data in, we
    // have to create a Transfer Buffer, copy memory into that, and then ask the GPU API to
    // copy the data from the transfer buffer into the vertex buffer.
    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(GBEVertex) * kNumVertices
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(context->device, &transferBufferCreateInfo);

    GBEVertex* vertexPtr = SDL_MapGPUTransferBuffer(context->device, transferBuffer, false);
    SDL_memcpy(vertexPtr, kVertices, sizeof(GBEVertex) * kNumVertices);
    SDL_UnmapGPUTransferBuffer(context->device, transferBuffer);
    vertexPtr = NULL;

    // Copy the data into the buffer
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(context->device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);
    SDL_GPUTransferBufferLocation sourceBuffer = {
        .transfer_buffer = transferBuffer,
        .offset = 0
    };
    SDL_GPUBufferRegion targetBuffer = {
        .buffer = vertexBuffer,
        .offset = 0,
        .size = sizeof(GBEVertex) * kNumVertices
    };

    SDL_UploadToGPUBuffer(copyPass, &sourceBuffer, &targetBuffer, true);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    SDL_ReleaseGPUTransferBuffer(context->device, transferBuffer);

    context->vertexBuffer = vertexBuffer;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void** appState, int argc, char** argv)
{
    SDL_SetAppMetadata("GPU by Example - Drawing a triangle", "0.0.1", "net.jonathanfischer.GpuByExamplePart1");

    AppContext* appContext;
    SDL_AppResult rc = AppContext_Init(&appContext);
    if (rc != SDL_APP_CONTINUE) {
        return rc;
    }
    *appState = appContext;

    rc = BuildPipeline(appContext);
    if (rc != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    rc = BuildVertexBuffer(appContext);
    return rc;
}

SDL_AppResult SDL_AppIterate(void* appState)
{
    AppContext* context = (AppContext*)appState;

    SDL_GPUCommandBuffer* cmdBuf;
    cmdBuf = SDL_AcquireGPUCommandBuffer(context->device);
    if (cmdBuf == NULL) {
        SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, context->window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("SDL_WaitAndAcquireGPUSwapchainTexture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture != NULL) {

        SDL_GPUColorTargetInfo targetInfo = {
            .texture = swapchainTexture,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = {0.12, 0.12, 0.12, 1}};

        SDL_GPURenderPass* renderPass;
        renderPass = SDL_BeginGPURenderPass(cmdBuf, &targetInfo, 1, NULL);

        // OK! Finally time to draw that triangle! We've begun our render pass as before, now we bind
        // the pipeline we've created, supply it with a vertex buffer, and tell it to draw some primitives,
        // telling it the number of vertices to work with.
        SDL_BindGPUGraphicsPipeline(renderPass, context->pipeline);

        // The API takes an array of vertex buffer pointers, not a single one.
        SDL_GPUBuffer* vertexBuffers[] = { context->vertexBuffer };
        SDL_BindGPUVertexStorageBuffers(renderPass, 0, vertexBuffers, 1);

        // We have 3 vertices to draw.
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(cmdBuf);

    // That's it for this frame.
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appState, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    // Nothing else to do, so just continue on with the next frame or event.
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appState, SDL_AppResult result)
{
    AppContext_Cleanup((AppContext*)appState);
}
