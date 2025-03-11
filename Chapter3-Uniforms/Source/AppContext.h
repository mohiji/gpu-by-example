//
//  AppContext.h
//  GpuByExample-Part1
//
//  Created by Jonathan Fischer on 3/10/25.
//

#ifndef AppContext_h
#define AppContext_h

#include <SDL3/SDL.h>

typedef struct GBE_AppContext {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_Storage* titleStorage;

    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* vertexBuffer;
} GBE_AppContext;

SDL_AppResult GBE_Init(GBE_AppContext** context);
void GBE_Cleanup(GBE_AppContext* context);

#endif /* AppContext_h */
