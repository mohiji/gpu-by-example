// PositionColor.vert.hlsl
//
// Takes in a 2-component position and 4-component color, outputs
// a 4-component position and the input color.
//
// Compile this for Direct3D 12 with:
// dxc -T vs_6_0 PositionColor.vert.hlsl -Fo PositionColor.vert.dxil
//
// or for Vulkan with:
// dxc -spirv -T vs_6_0 PositionColor.vert.hlsl -Fo PositionColor.vert.spv

struct InputVertex
{
    // 2-component (x, y) vertex position. The `: TEXCOORD0` annotation
    // (or semantic) help the API connect your vertex descriptions
    // up with the layout of this struct.
    //
    // See https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader#remarks
    // for why it's called TEXCOORD0.
    float2 Position : TEXCOORD0;

    // 4-component (r, g, b, a) color.
    float4 Color : TEXCOORD1;
};

struct OutputVertex
{
    // 4-component (x, y, z, w) vertex position. The SV_Position
    // semantic tells the driver that this is the field it wants
    // for a position.
    float4 Position : SV_Position;

    // Same 4-component color. In this case. the TEXCOORD0 will be
    // used to match this up with an input to the fragment shader.
    float4 Color : TEXCOORD0;
};

// The vertex shader takes a single input vertex and returns a
// single output vertex.
OutputVertex main(InputVertex input)
{
    OutputVertex output;

    // Take that (x, y) input position and add fixed z and w
    // components to it.
    output.Position = float4(input.Position, 0, 1);

    // Pass through the input color unchanged.
    output.Color = input.Color;

    return output;
}
