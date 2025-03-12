//
//  PositionColor.vert.glsl
//  Chapter2-DrawingPrimitives
//
//  Created by Jonathan Fischer on 3/12/25.
//
//  Just passes through a vertex with a position and color component.

// Which version of OpenGL Shading Language are we using. 
#version 450

// The input is our vertex structure:
//    struct GBEVertex {
//        float4 position;
//        float4 color;
//    };
//
// To represent that in OpenGL Shading Language, we define a couple
// of slots that match the memory layout of the structure.
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

// Our output from this shader is a vertex position, which
// we _don't_ have to explicitly define, and a color, which
// we do.
layout(location = 0) out vec4 fragColor;

// Unlike Metal and HLSL, OpenGL shader main functions don't
// return anything or take any arguments.
void main() {
    // The vertex position output is always named gl_Position
    gl_Position = inPosition;

    // And the color output we defined above
    fragColor = inColor;
}
