// This maps directly to the vertex structure in the host C
// program, with 4 32-bit float components for position and
// 4 32-bit float components for color.
struct Vertex
{
  // The SV_Position flag on this component tells the driver
  // that this field of the structure is the one that's used
  // as the clip-space position of the vertex at the end of
  // the pipeline.
  float4 Position : SV_Position;
  float4 Color : COLOR0;
};

Vertex vertex_main(Vertex input)
{
    return input;
}
