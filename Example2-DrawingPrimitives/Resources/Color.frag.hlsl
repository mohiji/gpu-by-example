// Color.frag.hlsl
//
// Takes in a color and returns it, easy-peasy.
//
// To compile this for Direct3D 12:
//     dxc -T ps_6_0 Color.frag.hlsl -Fo Color.frag.dxil
//
// or for Vulkan:
//     dxc -spirv -T ps_6_0 Color.frag.hlsl -Fo Color.frag.spv

// Semantics this time:
//  - TEXCOORD0 on the input matches it up witht he output from
//    the vertex shader.
//  - SV_Target0 means "use this as the final color"
float4 main(float4 Color : TEXCOORD0) : SV_Target0
{
    return Color;
}
