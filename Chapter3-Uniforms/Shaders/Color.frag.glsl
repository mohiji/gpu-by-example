//
//  Color.frag.glsl
//  Chapter2-DrawingPrimitives
//
//  Created by Jonathan Fischer on 3/12/25.
//
//  Similarly to PositionColor.vert.glsl, this just passes
//  through a single color

#version 450

// The input color we'll get from the vertex shader
layout(location = 0) in vec4 inColor;

// I don't know why there isn't a predefined location
// for this, like gl_Color or something.
layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
}
