//
//  Shaders.h
//  GpuByExample-Chapter3
//
//  Created by Jonathan Fischer on 3/10/25.
//
//  Wrapper around loading shaders.

#ifndef GpuByExample_Shaders_h
#define GpuByExample_Shaders_h

#include <SDL3/SDL.h>
#include "GBE_Context.h"

typedef struct GBE_LoadShaderInfo {
    const char* path;
    SDL_GPUShaderStage stage;
    const char* entryPoint;
    Uint32 samplerCount;
    Uint32 uniformBufferCount;
    Uint32 storageBufferCount;
    Uint32 storageTextureCount;
} GBE_LoadShaderInfo;

SDL_GPUShader* GBE_LoadShader(GBE_Context* context, const GBE_LoadShaderInfo* loadShaderInfo);

#endif /* GpuByExample_Shaders_h */
