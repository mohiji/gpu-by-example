//
//  Shaders.c
//  GpuByExample-Chapter3
//
//  Created by Jonathan Fischer on 3/10/25.
//
//  Wrapper around loading shaders. This is cribbed pretty heavily from Caleb
//  Cornet's SDL_GPU examples repo:
//  https://github.com/TheSpydog/SDL_gpu_examples/

#include "GBE_Shaders.h"

// Loading shaders is a little involved, in particular because we need to support
// 2 types of shaders (vertex and fragment/pixel) and 3 backends (Direct3D 12,
// Metal, and Vulkan).

SDL_GPUShader* GBE_LoadShader(
    SDL_Storage* storage,
    SDL_GPUDevice* device,
    const GBE_LoadShaderInfo* loadShaderInfo)
{
    if (loadShaderInfo->stage != SDL_GPU_SHADERSTAGE_VERTEX &&
        loadShaderInfo->stage != SDL_GPU_SHADERSTAGE_FRAGMENT) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Invalid shader stage!");
        return NULL;
    }

    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;

    char fullPath[256];
    const char* extraExtension = loadShaderInfo->stage == SDL_GPU_SHADERSTAGE_VERTEX ? "vert" : "frag";
    const char* entryPoint = "main";
    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%s.%s.spv", loadShaderInfo->path, extraExtension);
        format = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%s.%s.msl", loadShaderInfo->path, extraExtension);
        entryPoint = loadShaderInfo->stage == SDL_GPU_SHADERSTAGE_VERTEX ? "vertex_main" : "fragment_main";
        format = SDL_GPU_SHADERFORMAT_MSL;
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%s.%s.dxil", loadShaderInfo->path, extraExtension);
        format = SDL_GPU_SHADERFORMAT_DXIL;
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unrecognized backend shader format!");
        return NULL;
    }
    SDL_Log("Loading shader file %s...", fullPath);

    Uint64 codeSize;
    void* code;
    if (!SDL_GetStorageFileSize(storage, fullPath, &codeSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to determine size of file '%s': %s", fullPath, SDL_GetError());
        return NULL;
    }

    code = SDL_malloc(codeSize);
    if (!SDL_ReadStorageFile(storage, fullPath, code, codeSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to read file '%s': %s", fullPath, SDL_GetError());
        SDL_free(code);
        return NULL;
    }

    SDL_GPUShaderCreateInfo shaderInfo = {
        .code = code,
        .code_size = codeSize,
        .entrypoint = entryPoint,
        .format = format,
        .stage = loadShaderInfo->stage,
        .num_samplers = loadShaderInfo->samplerCount,
        .num_uniform_buffers = loadShaderInfo->uniformBufferCount,
        .num_storage_buffers = loadShaderInfo->storageBufferCount,
        .num_storage_textures = loadShaderInfo->storageTextureCount
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (shader == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create shader: %s", SDL_GetError());
    }

    SDL_free(code);
    return shader;
}
