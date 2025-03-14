//
//  RotatingTriangle.vert.glsl
//  Chapter3-DrawingPrimitives
//
//  Created by Jonathan Fischer on 3/12/25.
//

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

// For this example, we also pass in a rotation angle for the triangle.
layout(binding = 0) uniform GBE_Uniforms {
    float rotationAngle;
} Uniforms;

// Our output from this shader is a vertex position, which
// we _don't_ have to explicitly define, and a color, which
// we do.
layout(location = 0) out vec4 fragColor;

// Unlike Metal and HLSL, OpenGL shader main functions don't
// return anything or take any arguments.
void main() {
    // Rotate the incoming vertex's position around the origin.
    float sinTheta = sin(Uniforms.rotationAngle);
    float cosTheta = cos(Uniforms.rotationAngle);

    float x = inPosition.x * cosTheta - inPosition.y * sinTheta;
    float y = inPosition.y * cosTheta + inPosition.x * sinTheta;

    // The vertex position output is always named gl_Position
    gl_Position = vec4(x, y, 0, 1);

    // And the color output we defined above
    fragColor = inColor;
}
