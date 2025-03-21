//
//  GBE_Init.c
//  GBECommon
//
//  Created by Jonathan Fischer on 3/20/25.
//

#include <GBECommon/GBE_Init.h>

SDL_AppResult GBE_CommonInit(GBE_Context* appContext, const char* windowTitle)
{
    SDL_assert(appContext != NULL);
    SDL_assert(windowTitle != NULL);

    // Initialize the video and event subsystems
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUShaderFormat shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV |
                                        SDL_GPU_SHADERFORMAT_DXIL |
                                        SDL_GPU_SHADERFORMAT_METALLIB |
                                        SDL_GPU_SHADERFORMAT_MSL;

    SDL_GPUDevice* device = SDL_CreateGPUDevice(shaderFormats, true, NULL);
    if (device == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't not create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Using %s GPU implementation.", SDL_GetGPUDeviceDriver(device));

    SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow(windowTitle, 800, 600, windowFlags);

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

    appContext->window = window;
    appContext->device = device;
    appContext->titleStorage = storage;
    return SDL_APP_CONTINUE;
}

void GBE_Quit(GBE_Context* appContext)
{
    if (appContext->titleStorage != NULL) {
        SDL_CloseStorage(appContext->titleStorage);
    }

    if (appContext->device != NULL) {
        if (appContext->window != NULL) {
            SDL_ReleaseWindowFromGPUDevice(appContext->device, appContext->window);
            SDL_DestroyWindow(appContext->window);
        }

        SDL_DestroyGPUDevice(appContext->device);
    }

    SDL_Quit();
}

