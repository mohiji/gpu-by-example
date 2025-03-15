//
//  RotatingTriangle.vert.hlsl
//  Chapter3-Uniforms
//
//  Created by Jonathan Fischer on 3/14/25.
//

struct Input
{
    float4 Position : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

struct Output
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

cbuffer UniformBlock : register(b0, space1)
{
    float RotationAngle;
};

Output main(Input input)
{
    // Rotate the incoming vertex's position around the origin.
    float sinTheta = sin(RotationAngle);
    float cosTheta = cos(RotationAngle);

    float x = input.Position.x * cosTheta - input.Position.y * sinTheta;
    float y = input.Position.y * cosTheta + input.Position.x * sinTheta;

    Output output;
    output.Color = input.Color;
    output.Position = float4(x, y, 0, 1);
    return output;
}

