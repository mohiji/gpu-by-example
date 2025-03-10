//
//  AppContext.h
//  GpuByExample-Part1
//
//  Created by Jonathan Fischer on 3/10/25.
//

#ifndef AppContext_h
#define AppContext_h

#include <SDL3/SDL.h>

// For our AppContext structure, we're adding a graphics pipeline and a buffer in GPU
// memory to hold the vertices for the triangle we're drawing.
typedef struct AppContext {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_Storage* titleStorage;

    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertexBuffer;
} AppContext;

SDL_AppResult AppContext_Init(AppContext** context);
void AppContext_Cleanup(AppContext* context);

#endif /* AppContext_h */
