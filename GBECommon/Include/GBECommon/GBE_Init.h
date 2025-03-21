//
//  GBE_Init.h
//  GBECommon
//
//  Created by Jonathan Fischer on 2025-03-20.
//
//  Boilerplate SDL initialization / cleanup.

#ifndef GBE_Init_h
#define GBE_Init_h

#include "GBE_Context.h"

SDL_AppResult GBE_CommonInit(GBE_Context* appContext, const char* windowTitle);
void          GBE_Quit(GBE_Context* appContext);

#endif /* GBE_Init_h */
