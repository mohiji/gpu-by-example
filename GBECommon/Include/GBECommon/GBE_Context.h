//
//  GBE_Context.h
//  GBECommon
//
//  Created by Jonathan Fischer on 3/21/25.
//
//  Just defines a structure used to carry around stuff we need.

#ifndef GBE_Context_h
#define GBE_Context_h

#include <SDL3/SDL.h>

typedef struct GBE_Context {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_Storage* titleStorage;
} GBE_Context;

#endif /* GBE_Context_h */
