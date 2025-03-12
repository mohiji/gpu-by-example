// Input from the host program. This matches the GBEVertex structure defined
// in main.c
struct GBEVertex
{
    float4 position : TEXCOORD0;
    float4 color : TEXCOORD1;
};

// Output from the vertex program. Yes, it's the same as the input, with
// semantic tags added to tell the driver how to link this vertex shader
// together with the fragment/pixel shader.
struct Output
{
    float4 position : SV_Position;
    float4 color : TEXCOORD0;
};

Output vertex_main(GBEVertex input)
{
    Output output;
    output.position = input.position;
    output.color = input.color;
    return output;
}

