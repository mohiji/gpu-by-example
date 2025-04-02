// SpinningCube.vert.glsl
//
// Takes in a full 4-component vertex position and transforms it
// using a matrix provided in a Uniform Block.
//
// To compile this for Vulkan:
//   glslang -V SpinningCube.vert.glsl -o SpinningCube.vert.spv

// Which version of OpenGL Shading Language are we using. 
#version 450

// The input is our vertex structure:
//    struct Vertex {
//        float4 position;
//        float4 color;
//    };
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

// The vertex position output is a special variable always named
// gl_Position; we don't define that one. We _do_ need to define
// anything else we're returning. In this case, it's the
// 4-component vertex color:
layout(location = 0) out vec4 fragColor;

// We provide a transformation matrix via a Uniform Block. Fun
// stuff going on here:
//
// - This is the first uniform buffer slot, so binding = 0.
// - To be completely honest, I don't understand why I need
//   set = 1. I added it after comparing the assembly output
//   of this file using glslang vs the HLSL equivalent file
//   using dxc. Without set = 1, the program will crash.
layout(binding = 0, set = 1) uniform TransformBlock
{
    mat4 modelViewProjection;
} transforms;

// Unlike Metal and HLSL, OpenGL shader main functions don't
// return anything or take any arguments.
void main() {
    // Transform the vertex by the provided transform matrix.
    gl_Position = transforms.modelViewProjection * inPosition;

    // And the color output we defined above
    fragColor = inColor;
}
