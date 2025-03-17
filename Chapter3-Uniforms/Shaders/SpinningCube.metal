//
//  SpinningCube.metal
//  Chapter3-Uniforms
//
//  Created by Jonathan Fischer on 3/17/25.
//

#include <metal_stdlib>
using namespace metal;

// Sticking with the same Vertex structure as last time.
struct GBEVertex
{
    float4 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
};

// The calling program provides a full Model View Projection matrix,
// all we need to do in here is apply it.
struct Uniforms
{
    float4x4 modelViewProjectionMatrix;
};

struct OutputVertex
{
    float4 position [[position]];
    float4 color;
};

vertex OutputVertex vertex_main(
  GBEVertex vertex_in [[stage_in]],
  constant Uniforms* uniforms [[buffer(0)]]
)
{
    OutputVertex vertex_out;
    vertex_out.position = uniforms->modelViewProjectionMatrix * vertex_in.position;
    vertex_out.color = vertex_in.color;
    return vertex_out;
}

fragment float4 fragment_main(OutputVertex vertex_in [[stage_in]])
{
    return vertex_in.color;
}
