// I honestly don't know why/if these are necessary ATM.
#include <metal_stdlib>
using namespace metal;

// This maps directly to the vertex structure in the host C
// program, with 4 32-bit float components for position and
// 4 32-bit float components for color.
struct Vertex
{
  // The [[position]] flag on this component tells the driver
  // that this field of the structure is the one that's used
  // as the clip-space position of the vertex at the end of
  // the pipeline.
  float4 position [[position]];

  // color is left unadorned: it doesn't have any special
  // meaning attached, only what we use it for.
  float4 color;
};

// The vertex shading function. It's first tagged `vertex`
// to indicate that it's for the vertex shading stage,
// rather than the fragment stage. Then we have a normal
// return type + function name + parameter list.
vertex Vertex vertex_main(
  // Lots to unpack on this parameter declaration:
  // - const, because this input is read-only
  // - device, because it's from the device address space.
  //   "The device address space name refers to buffer
  //    memory objects allocated from the device memory pool
  //    that are both readable and writeable." (from Apple's
  //    Metal Language Specification)
  // - Vertex* vertices is the normal type + parameter name
  // - [[buffer(0)]] tells the driver that this parameter
  //   maps to the first buffer bound to this shader
  const device Vertex* vertices [[buffer(0)]],

  // This parameter tells the shading function which vertex
  // to look up in the vertex parameter. We're drawing
  // simple primitives, so this will just increase from 0
  // to the number of vertices provided in our draw call,
  // but later when we use index buffers for rendering the
  // vertex ids will come from there.
  uint vid [[vertex_id]]
)
{
  // For this demonstration our vertices are already in clip
  // space so we can just return the vertex directly.
  return vertices[vid];
}

// The fragment shading function. The fragment shading
// function always returns the final color of the pixel
// being rendered.
fragment float4 fragment_main(
  // [[stage_in]] means this argument will come from the
  // output of the shading function. Note that it's not a
  // _direct_ output, because, the GPU will interpolate
  // vertex values over the pixels being rendered. We only
  // supply the 3 vertices, it has to interpolate their
  // values across the rest of the image.
  Vertex inVertex [[stage_in]]
)
{
  return inVertex.color;
}
