//
//  PositionColor.vert.hlsl
//  Chapter2-DrawingPrimitives
//
//  Created by Jonathan Fischer on 3/12/25.
//
//  Just passes through a vertex with a position and color component.


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

Output main(Input input)
{
    Output output;
    output.Color = input.Color;
    output.Position = input.Position;
    return output;
}
