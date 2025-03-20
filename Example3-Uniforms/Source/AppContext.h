//
//  AppContext.h
//  GpuByExample-Part1
//
//  Created by Jonathan Fischer on 3/10/25.
//

#ifndef AppContext_h
#define AppContext_h

#include <SDL3/SDL.h>
#include "GBE_3DMath.h"

typedef struct GBE_Uniforms {
    GBE_Matrix4x4 modelViewProjectionMatrix;
} GBE_Uniforms;

typedef struct GBE_AppContext {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_Storage* titleStorage;

    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;

    Uint64 lastFrameTime;
    Uint64 elapsedTime;
    float rotationX;
    float rotationY;

    GBE_Uniforms uniforms;
} GBE_AppContext;

SDL_AppResult GBE_Init(GBE_AppContext** context);
void GBE_Cleanup(GBE_AppContext* context);

#endif /* AppContext_h */
