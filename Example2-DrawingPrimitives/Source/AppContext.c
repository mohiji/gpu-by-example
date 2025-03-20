//
//  AppContext.c
//  GpuByExample-Part1
//
//  Created by Jonathan Fischer on 3/10/25.
//

#include "AppContext.h"

SDL_AppResult GBE_Init(GBE_AppContext** appContext)
{
    // Initialize the video and event subsystems
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUShaderFormat shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV |
                                        SDL_GPU_SHADERFORMAT_DXIL |
                                        SDL_GPU_SHADERFORMAT_MSL;

    SDL_GPUDevice* device = SDL_CreateGPUDevice(shaderFormats, false, NULL);
    if (device == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,  "Couldn't not create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow("GPU by Example - Drawing Primitives", 800, 600, windowFlags);

    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // SDL has some functionality to help us locate a game's resource files; here I'm using
    // their Storage abstraction.
    SDL_Storage* storage = SDL_OpenTitleStorage(NULL, 0);
    if (storage == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to open title storage: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // I don't think this is necessary on desktop platforms, but on some devices
    // the storage instance might not be available immediately, so we busy wait
    // until it is.
    while (!SDL_StorageReady(storage)) {
        SDL_Delay(1);
    }

    // Switched to calloc for safety's sake; now I know it's zero-initialized.
    GBE_AppContext* context = SDL_calloc(1, sizeof(GBE_AppContext));
    context->window = window;
    context->device = device;
    context->titleStorage = storage;

    *appContext = context;
    return SDL_APP_CONTINUE;
}

void GBE_Cleanup(GBE_AppContext* context)
{
    if (context->titleStorage != NULL) {
        SDL_CloseStorage(context->titleStorage);
    }

    if (context->device != NULL) {
        if (context->window != NULL) {
            SDL_ReleaseWindowFromGPUDevice(context->device, context->window);
            SDL_DestroyWindow(context->window);
        }

        if (context->pipeline != NULL) {
            SDL_ReleaseGPUGraphicsPipeline(context->device, context->pipeline);
        }

        if (context->vertexBuffer != NULL) {
            SDL_ReleaseGPUBuffer(context->device, context->vertexBuffer);
        }

        SDL_DestroyGPUDevice(context->device);
    }

    SDL_free(context);
    SDL_Quit();
}
