// Color.frag.glsl
//
// Takes in a color and returns it
//
// To compile this for Vulkan:
//     glslang -V Color.frag.glsl -o Color.frag.spv

#version 450

// The input color we'll get from the vertex shader
layout(location = 0) in vec4 inColor;

// Unlike gl_Position in the vertex shader, we need
// to name an output variable for the color.
layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
}
