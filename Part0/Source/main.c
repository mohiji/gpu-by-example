//
//  main.c
//  GpuByExample
//
//  Created by Jonathan Fischer on 3/4/25.
//

// Before including SDL_main.h, define this to enable the new application lifecycle stuff.
#define SDL_MAIN_USE_CALLBACKS

// Pull in SDL3, obviously.
#include <SDL3/SDL.h>

// And only include SDL_main.h in the file where you define your main function(s).
#include <SDL3/SDL_main.h>

// Our application context structure. For now, we have our
// window and GPU device, but this'll grow in later posts.
typedef struct AppContext {
    SDL_Window* window;
    SDL_GPUDevice* device;
} AppContext;

// SDL_AppInit is the first function that will be called.
// This is where you initialize SDL, load resources that
// your game will need from the start, etc.
SDL_AppResult SDL_AppInit(void** appState, int argc, char** argv)
{
    // This isn't strictly necessary, but if you provide
    // a little bit of metadata here SDL will use it in
    // things like the About window on macOS.
    SDL_SetAppMetadata("GPU by Example - Scaffolding", "0.0.1", "net.jonathanfischer.GpuByExamplePart0");

    // Initialize the video and event subsystems
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // This surprised me, but you create the GPU device first, then the window.
    // You'll need to tell the API up front what shader languages you plan on
    // supporting.
    SDL_GPUShaderFormat shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV |
                                        SDL_GPU_SHADERFORMAT_DXIL |
                                        SDL_GPU_SHADERFORMAT_MSL;

    SDL_GPUDevice* device = SDL_CreateGPUDevice(shaderFormats, false, NULL);
    if (device == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't not create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Now go ahead and create a window. I'm creating a high pixel density
    // window because without that, I was getting blurry text on macOS. (text
    // comes in a later post, promise.)
    SDL_WindowFlags windowFlags =
        SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE;
    SDL_Window* window =
        SDL_CreateWindow("GPU by Example - Scaffolding", 800, 600, windowFlags);

    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Then bind the window and GPU device together
    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // By default, SDL GPU enables VSYNC, which is generally what I want. If you
    // want to change it, now is the time to do that; look at
    // SDL_SetGPUSwapchainParameters in the documentation.
    // https://wiki.libsdl.org/SDL3/SDL_SetGPUSwapchainParameters

    // Last up, let's create our context object and store pointers to our window
    // and GPU device. We stick it in the appState argument passed to this
    // function and SDL will provide it in later calls.
    AppContext* context = SDL_malloc(sizeof(AppContext));
    context->window = window;
    context->device = device;
    *appState = context;

    // And that's it for initialization.
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appState)
{
    // Our AppContext instance is passed in through the appState pointer.
    AppContext* context = (AppContext*)appState;

    // Generally speaking, this is where you'd track frame times, update your game state, etc.
    // I'll be doing that in later posts.

    // Once you're ready to start drawing, begin by grabbing a command buffer and a reference to the
    // swapchain texture.
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

    // With the command buffer and swapchain texture in hand, we can begin and end our render pass
    if (swapchainTexture != NULL) {
        // There are a lot more options you can set for a render pass, see SDL_GPUColorTargetInfo in the SDL
        // documentation for more.
        // https://wiki.libsdl.org/SDL3/SDL_GPUColorTargetInfo
        SDL_GPUColorTargetInfo targetInfo = {
            // The texture that we're drawing in to
            .texture = swapchainTexture,

            // Whether to cycle that texture.
            // See https://moonside.games/posts/sdl-gpu-concepts-cycling/ for more info
            .cycle = true,

            // Clear the texture to a known color before drawing
            .load_op = SDL_GPU_LOADOP_CLEAR,

            // Keep the rendered output
            .store_op = SDL_GPU_STOREOP_STORE,

            // And here's the clear color, a nice green.
            .clear_color = {0.16, 0.47, 0.34, 1}};

        // Begin and end the render pass. With no drawing commands, this will clear the swapchain texture
        // to the color provided above and nothing else.
        SDL_GPURenderPass* renderPass;
        renderPass = SDL_BeginGPURenderPass(cmdBuf, &targetInfo, 1, NULL);
        SDL_EndGPURenderPass(renderPass);
    }

    // And finally, submit the command buffer for drawing. The driver will take over at this point
    // and do all the rendering we've asked it to.
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    // That's it for this frame.
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appState, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        // SDL_APP_SUCCESS means we're making a clean exit.
        // SDL_APP_FAILURE would mean something went wrong.
        return SDL_APP_SUCCESS;
    }

    // Nothing else to do, so just continue on with the next frame or event.
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appState, SDL_AppResult result)
{
    AppContext* context = (AppContext*)appState;

    SDL_ReleaseWindowFromGPUDevice(context->device, context->window);
    SDL_DestroyWindow(context->window);
    SDL_DestroyGPUDevice(context->device);

    SDL_Quit();
}
