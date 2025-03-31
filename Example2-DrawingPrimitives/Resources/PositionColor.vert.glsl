//  PositionColor.vert.glsl
//
// Takes in a 2-component position and 4-component color, outputs
// a 4-component position and the input color.
//
// To compile this for Vulkan:
//   glslang -V PositionColor.vert.glsl -o PositionColor.vert.spv

// Which version of OpenGL Shading Language are we using. 
#version 450

// The input is our vertex structure:
//    struct Vertex {
//        float2 position;
//        float4 color;
//    };
//
// To represent that in OpenGL Shading Language, we define a couple
// of slots that match the memory layout of the structure.
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

// The vertex position output is a special variable always named
// gl_Position; we don't define that one. We _do_ need to define
// anything else we're returning. In this case, it's the
// 4-component vertex color:
layout(location = 0) out vec4 fragColor;

// Unlike Metal and HLSL, OpenGL shader main functions don't
// return anything or take any arguments.
void main() {
    // The vertex position output is always named gl_Position
    gl_Position = vec4(inPosition, 0, 1);

    // And the color output we defined above
    fragColor = inColor;
}
