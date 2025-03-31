//
//  main.c
//  GpuByExample
//
//  Created by Jonathan Fischer on 3/6/25.
//

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// I'm trying to keep the interesting / new parts of each example in the
// example's source file and move things we've already seen out of the way
// to keep the file easy to read. Common stuff has been moved out to GBECommon.
#include <GBECommon/GBE_Init.h>

// Loading shaders is, unfortunately, somewhat involved. I'll cover further
// what's going on here in a follow-up post.
#include <GBECommon/GBE_Shaders.h>

// Our app context structure this time needs to keep track of a pipeline and
// vertex buffer.
typedef struct AppContext {
    GBE_Context common;

    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertexBuffer;
} AppContext;

// This time through we're actually drawing something, so we need _geometry_. And for
// that, we need to specify exactly what we're going to give the GPU. For now, that's
// a position and color.
typedef struct Vertex {
  float x, y;
  float r, g, b, a;
} Vertex;

// Now we need the actual triangle geometry. I'm cheating a bit here and choosing to
// draw a triangle that's already defined in clip space, so we don't need to worry
// about any math yet. (Or more specifically, providing a transformation to the vertex
// shader.)
static const size_t kNumVertices = 3;

const Vertex kVertices[] = {
  {  0.0,  0.5, 1.0, 0.0, 0.0, 1.0 },
  { -0.5, -0.5, 0.0, 1.0, 0.0, 1.0 },
  {  0.5, -0.5, 0.0, 0.0, 1.0, 1.0 }
};

static SDL_AppResult BuildPipeline(AppContext* context)
{
    GBE_LoadShaderInfo vertexShaderInfo = {
        .path = "PositionColor",
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .samplerCount = 0,
        .uniformBufferCount = 0,
        .storageBufferCount = 0,
        .storageTextureCount = 0
    };

    SDL_GPUShader* vertexShader = GBE_LoadShader(&context->common, &vertexShaderInfo);
    if (vertexShader == NULL) {
        return SDL_APP_FAILURE;
    }

    GBE_LoadShaderInfo fragmentShaderInfo = {
        .path = "Color",
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .samplerCount = 0,
        .uniformBufferCount = 0,
        .storageBufferCount = 0,
        .storageTextureCount = 0
    };

    SDL_GPUShader* fragmentShader = GBE_LoadShader(&context->common, &fragmentShaderInfo);
    if (fragmentShader == NULL) {
        SDL_ReleaseGPUShader(context->common.device, vertexShader);
        return SDL_APP_FAILURE;
    }

    // Lots of stuff here! Let's take it one chunk at a time. First, we describe the
    // render target, which includes both a target pixel format and how we want to
    // blend colors into it.
    SDL_GPUTextureFormat targetFormat = SDL_GetGPUSwapchainTextureFormat(context->common.device, context->common.window);

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

    // We need to tell the pipeline exactly what shape our input data is going to be
    // in. Metal will let us get away without this and still work fine! Vulkan and
    // Direct3D12 will not.
    SDL_GPUVertexInputState vertexInputState = {
        .num_vertex_buffers = 1,
        .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
            .slot = 0,
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
            .pitch = sizeof(Vertex)
        }},
        .num_vertex_attributes = 2,
        .vertex_attributes = (SDL_GPUVertexAttribute[]){{
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .location = 0,
            .offset = 0
        }, {
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .location = 1,
            .offset = sizeof(float) * 2
        }}
    };

    // That should be enough to create a pipeline! We provide the target info and
    // rasterizer state defined above, we want to draw triangles, and we want to
    // do it using the vertex and fragment shaders we loaded.
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .target_info = targetInfo,
        .rasterizer_state = rasterizerState,
        .vertex_input_state = vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader
    };

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(context->common.device, &pipelineCreateInfo);
    if (pipeline == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create graphics pipeline: %s", SDL_GetError());
    }

    // Now that the pipeline has been created, it's holding on to references to the shaders, so we
    // don't need to keep them around anymore. (I think they're reference counted, and the pipeline
    // retained them.)
    SDL_ReleaseGPUShader(context->common.device, vertexShader);
    SDL_ReleaseGPUShader(context->common.device, fragmentShader);

    // Store the created pipeline (or the NULL if it failed) in our application context and be done.
    context->pipeline = pipeline;
    return pipeline != NULL ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
}

static SDL_AppResult BuildVertexBuffer(AppContext* context)
{
    // Creating a vertex buffer is easy: all we really need to tell it is what
    // we're going to use it for and how much space to allocate.
    SDL_GPUBufferCreateInfo bufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = (Uint32)(sizeof(Vertex) * kNumVertices)
    };
    SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(context->common.device, &bufferCreateInfo);

    // Getting data _into_ the vertex buffer is way more work. We can't just copy data in, we
    // have to create a Transfer Buffer, copy memory into that, and then ask the GPU API to
    // copy the data from the transfer buffer into the vertex buffer.
    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (Uint32)(sizeof(Vertex) * kNumVertices)
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(context->common.device, &transferBufferCreateInfo);

    Vertex* vertexPtr = SDL_MapGPUTransferBuffer(context->common.device, transferBuffer, false);
    SDL_memcpy(vertexPtr, kVertices, sizeof(Vertex) * kNumVertices);
    SDL_UnmapGPUTransferBuffer(context->common.device, transferBuffer);
    vertexPtr = NULL;

    // Copy the data into the buffer
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(context->common.device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);
    SDL_GPUTransferBufferLocation sourceBuffer = {
        .transfer_buffer = transferBuffer,
        .offset = 0
    };
    SDL_GPUBufferRegion targetBuffer = {
        .buffer = vertexBuffer,
        .offset = 0,
        .size = (Uint32)(sizeof(Vertex) * kNumVertices)
    };

    SDL_UploadToGPUBuffer(copyPass, &sourceBuffer, &targetBuffer, true);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    SDL_ReleaseGPUTransferBuffer(context->common.device, transferBuffer);

    context->vertexBuffer = vertexBuffer;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void** appState, int argc, char** argv)
{
    SDL_SetAppMetadata("GPU by Example - Drawing Primitives", "0.0.1", "net.jonathanfischer.GpuByExample2");

    AppContext* appContext = SDL_calloc(sizeof(AppContext), 1);
    SDL_AppResult rc = GBE_CommonInit(&appContext->common, "GPU by Example - Drawing Primitives");
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
    cmdBuf = SDL_AcquireGPUCommandBuffer(context->common.device);
    if (cmdBuf == NULL) {
        SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, context->common.window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("SDL_WaitAndAcquireGPUSwapchainTexture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture != NULL) {
        SDL_GPUColorTargetInfo targetInfo = {
            .texture = swapchainTexture,
            .cycle = false,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = {0.12f, 0.12f, 0.12f, 1.0f}};

        SDL_GPURenderPass* renderPass;
        renderPass = SDL_BeginGPURenderPass(cmdBuf, &targetInfo, 1, NULL);

        // OK! Finally time to draw that triangle! We've begun our render pass as before, now we bind
        // the pipeline we've created, supply it with a vertex buffer, and tell it to draw some primitives,
        // telling it the number of vertices to work with.
        SDL_BindGPUGraphicsPipeline(renderPass, context->pipeline);

        // The API takes an array of vertex buffer pointers, not a single one.
        SDL_GPUBufferBinding vertexBufferBinding = {
            .buffer = context->vertexBuffer,
            .offset = 0
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

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

    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appState, SDL_AppResult result)
{
    AppContext* context = (AppContext*)appState;
    if (context->vertexBuffer != NULL) {
        SDL_ReleaseGPUBuffer(context->common.device, context->vertexBuffer);
    }

    if (context->pipeline != NULL) {
        SDL_ReleaseGPUGraphicsPipeline(context->common.device, context->pipeline);
    }

    GBE_Quit(&context->common);
    SDL_free(context);
}
