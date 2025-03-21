//
//  main.c
//  GpuByExample
//
//  Created by Jonathan Fischer on 3/6/25.
//

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <GBECommon/GBE_3DMath.h>
#include <GBECommon/GBE_Init.h>
#include <GBECommon/GBE_Context.h>
#include <GBECommon/GBE_Shaders.h>

// Why in the world does Visual Studio not define M_PI and the like
// without this extra bit?
#define _USE_MATH_DEFINES
#include <math.h> // for M_PI

typedef struct Vertex {
    GBE_Vector4 position;
    GBE_Vector4 color;
} Vertex;

typedef struct Uniforms {
    GBE_Matrix4x4 modelViewProjectionMatrix;
} Uniforms;

typedef struct AppContext {
    GBE_Context context;

    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;

    Uint64 lastFrameTime;
    Uint64 elapsedTime;
    float rotationX;
    float rotationY;

    Uniforms uniforms;
} AppContext;

static const size_t kNumVertices = 8;
static const size_t kNumIndices = 36;
static const Vertex kVertices[kNumVertices] = {
    { .position = { -1,  1,  1, 1}, .color = { 0, 1, 1, 1 } },
    { .position = { -1, -1,  1, 1}, .color = { 0, 0, 1, 1 } },
    { .position = {  1, -1,  1, 1}, .color = { 1, 0, 1, 1 } },
    { .position = {  1,  1,  1, 1}, .color = { 1, 1, 1, 1 } },
    { .position = { -1,  1, -1, 1}, .color = { 0, 1, 0, 1 } },
    { .position = { -1, -1, -1, 1}, .color = { 0, 0, 0, 1 } },
    { .position = {  1, -1, -1, 1}, .color = { 1, 0, 0, 1 } },
    { .position = {  1,  1, -1, 1}, .color = { 1, 1, 0, 1 } }
};

static const Uint16 kIndices[kNumIndices] = {
    3, 2, 6, 6, 7, 3,
    4, 5, 1, 1, 0, 4,
    4, 0, 3, 3, 7, 4,
    1, 5, 6, 6, 2, 1,
    0, 1, 2, 2, 3, 0,
    7, 6, 5, 5, 4, 7
};

static SDL_AppResult BuildPipeline(AppContext* context)
{
    GBE_LoadShaderInfo vertexShaderInfo = {
        .path = "SpinningCube",
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .samplerCount = 0,
        .uniformBufferCount = 1,
        .storageBufferCount = 0,
        .storageTextureCount = 0
    };

    SDL_GPUShader* vertexShader = GBE_LoadShader(&context->context, &vertexShaderInfo);
    if (vertexShader == NULL) {
        return SDL_APP_FAILURE;
    }

    GBE_LoadShaderInfo fragmentShaderInfo = {
        .path = "SpinningCube",
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .samplerCount = 0,
        .uniformBufferCount = 0,
        .storageBufferCount = 0,
        .storageTextureCount = 0
    };

    SDL_GPUShader* fragmentShader = GBE_LoadShader(&context->context, &fragmentShaderInfo);
    if (fragmentShader == NULL) {
        SDL_ReleaseGPUShader(context->context.device, vertexShader);
        return SDL_APP_FAILURE;
    }

    // Lots of stuff here! Let's take it one chunk at a time. First, we describe the
    // render target, which includes both a target pixel format and how we want to
    // blend colors into it.
    SDL_GPUTextureFormat targetFormat = SDL_GetGPUSwapchainTextureFormat(context->context.device, context->context.window);

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
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .location = 0,
            .offset = 0
        }, {
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .location = 1,
            .offset = sizeof(float) * 4
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

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(context->context.device, &pipelineCreateInfo);
    if (pipeline == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create graphics pipeline: %s", SDL_GetError());
    }

    // Now that the pipeline has been created, it's holding on to references to the shaders, so we
    // don't need to keep them around anymore. (I think they're reference counted, and the pipeline
    // retained them.)
    SDL_ReleaseGPUShader(context->context.device, vertexShader);
    SDL_ReleaseGPUShader(context->context.device, fragmentShader);

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
    SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(context->context.device, &bufferCreateInfo);

    // Getting data _into_ the vertex buffer is way more work. We can't just copy data in, we
    // have to create a Transfer Buffer, copy memory into that, and then ask the GPU API to
    // copy the data from the transfer buffer into the vertex buffer.
    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (Uint32)(sizeof(Vertex) * kNumVertices)
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(context->context.device, &transferBufferCreateInfo);

    Vertex* vertexPtr = SDL_MapGPUTransferBuffer(context->context.device, transferBuffer, false);
    SDL_memcpy(vertexPtr, kVertices, sizeof(Vertex) * kNumVertices);
    SDL_UnmapGPUTransferBuffer(context->context.device, transferBuffer);
    vertexPtr = NULL;

    // Copy the data into the buffer
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(context->context.device);
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

    SDL_ReleaseGPUTransferBuffer(context->context.device, transferBuffer);

    context->vertexBuffer = vertexBuffer;
    return SDL_APP_CONTINUE;
}

static SDL_AppResult BuildIndexBuffer(AppContext* context)
{
    // This is pretty much the same thing as BuildVertexBuffer, except we're working with
    // Uint16 instead of Vertex.
    SDL_GPUBufferCreateInfo createInfo = {
        // NOTE: Index buffers use a different usage type than vertex buffers
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(Uint16) * kNumIndices
    };
    SDL_GPUBuffer* indexBuffer = SDL_CreateGPUBuffer(context->context.device, &createInfo);

    // Copying data in works the same way.
    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(Uint16) * kNumIndices
    };
    SDL_GPUTransferBuffer* indexTransferBuffer = SDL_CreateGPUTransferBuffer(context->context.device, &transferBufferCreateInfo);

    // Copy our static indices over into GPU memory.
    Uint16* indexPtr = SDL_MapGPUTransferBuffer(context->context.device, indexTransferBuffer, false);
    SDL_memcpy(indexPtr, kIndices, sizeof(Uint16) * kNumIndices);
    SDL_UnmapGPUTransferBuffer(context->context.device, indexTransferBuffer);
    indexPtr = NULL;

    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(context->context.device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);
    SDL_GPUTransferBufferLocation sourceBuffer = {
        .transfer_buffer = indexTransferBuffer,
        .offset = 0
    };
    SDL_GPUBufferRegion targetBuffer = {
        .buffer = indexBuffer,
        .offset = 0,
        .size = (Uint32)(sizeof(Uint16) * kNumIndices)
    };

    SDL_UploadToGPUBuffer(copyPass, &sourceBuffer, &targetBuffer, true);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    SDL_ReleaseGPUTransferBuffer(context->context.device, indexTransferBuffer);
    context->indexBuffer = indexBuffer;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void** appState, int argc, char** argv)
{
    SDL_SetAppMetadata("GPU by Example - Uniforms", "0.0.1", "net.jonathanfischer.GpuByExample3");

    AppContext* appContext = SDL_calloc(1, sizeof(AppContext));
    SDL_AppResult rc = GBE_CommonInit(&appContext->context, "GPU by Example - Uniforms");
    if (rc != SDL_APP_CONTINUE) {
        return rc;
    }
    *appState = appContext;

    rc = BuildPipeline(appContext);
    if (rc != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    rc = BuildVertexBuffer(appContext);
    if (rc != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    rc = BuildIndexBuffer(appContext);
    appContext->lastFrameTime = SDL_GetTicks();

    return rc;
}

static void frameStep(AppContext* appContext)
{
    // Calculate how many milliseconds have passed since the last time we did this.
    Uint64 currentFrameTime = SDL_GetTicks();
    Uint64 deltaFrameTime = currentFrameTime - appContext->lastFrameTime;

    // If somehow we're running too slowly to maintain a decent frame time (or more likely,
    // we spent time paused in a debugger), cap the amount of time we consider to have
    // passed to 1/5 of a second so we don't have any weird jumps.
    if (deltaFrameTime > 200) {
        deltaFrameTime = 200;
    }

    // And keep track of how much time has passed overall; we'll use it below when
    // calculating how much to scale the cube by.
    appContext->elapsedTime += deltaFrameTime;

    // Keep the cube spinning along
    float dt = deltaFrameTime / 1000.0f;
    appContext->rotationX += dt * (M_PI / 2);
    appContext->rotationY += dt * (M_PI / 3);

    float scaleFactor = sinf(5 * appContext->elapsedTime / 1000.0f) * 0.25 + 1;
    GBE_Vector3 xAxis = { 1, 0, 0 };
    GBE_Vector3 yAxis = { 0, 1, 0 };
    GBE_Matrix4x4 xRot = GBE_Matrix4x4RotateAxisAngle(xAxis, appContext->rotationX);
    GBE_Matrix4x4 yRot = GBE_Matrix4x4RotateAxisAngle(yAxis, appContext->rotationY);
    GBE_Matrix4x4 scale = GBE_Matrix4x4UniformScale(scaleFactor);
    GBE_Matrix4x4 modelMatrix = GBE_Matrix4x4Multiply(GBE_Matrix4x4Multiply(xRot, yRot), scale);

    GBE_Vector3 cameraTranslation = { 0, 0, -5 };
    GBE_Matrix4x4 viewMatrix = GBE_Matrix4x4Translation(cameraTranslation);

    int viewportWidth, viewportHeight;
    SDL_GetWindowSize(appContext->context.window, &viewportWidth, &viewportHeight);
    float aspect = (float)viewportWidth / viewportHeight;
    float fov = (2 * M_PI) / 5;
    float near = 1;
    float far = 100;
    GBE_Matrix4x4 projectionMatrix = GBE_Matrix4x4Perspective(aspect, fov, near, far);

    appContext->uniforms.modelViewProjectionMatrix = GBE_Matrix4x4Multiply(GBE_Matrix4x4Multiply(modelMatrix, viewMatrix), projectionMatrix);

    appContext->lastFrameTime = currentFrameTime;
}

SDL_AppResult SDL_AppIterate(void* appState)
{
    AppContext* context = (AppContext*)appState;
    frameStep(context);

    SDL_GPUCommandBuffer* cmdBuf;
    cmdBuf = SDL_AcquireGPUCommandBuffer(context->context.device);
    if (cmdBuf == NULL) {
        SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, context->context.window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("SDL_WaitAndAcquireGPUSwapchainTexture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture != NULL) {

        SDL_GPUColorTargetInfo targetInfo = {
            .texture = swapchainTexture,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = {0.12f, 0.12f, 0.12f, 1.0f}};

        SDL_GPURenderPass* renderPass;
        renderPass = SDL_BeginGPURenderPass(cmdBuf, &targetInfo, 1, NULL);
        SDL_BindGPUGraphicsPipeline(renderPass, context->pipeline);
        SDL_PushGPUVertexUniformData(cmdBuf, 0, &(context->uniforms), sizeof(Uniforms));

        // The API takes an array of vertex buffer pointers, not a single one.
        SDL_GPUBufferBinding vertexBufferBinding = {
            .buffer = context->vertexBuffer,
            .offset = 0
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

        SDL_GPUBufferBinding indexBinding = {
            .buffer = context->indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_DrawGPUIndexedPrimitives(renderPass, kNumIndices, 1, 0, 0, 0);
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
    AppContext* context = (AppContext*)appState;

    if (context->pipeline != NULL) {
        SDL_ReleaseGPUGraphicsPipeline(context->context.device, context->pipeline);
    }

    if (context->vertexBuffer != NULL) {
        SDL_ReleaseGPUBuffer(context->context.device, context->vertexBuffer);
    }

    GBE_Quit(&context->context);
    SDL_free(context);
}
