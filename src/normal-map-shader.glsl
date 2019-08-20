#include "common.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) readonly uniform image2D displacement;
layout (binding = 1, rgba32f) writeonly uniform image2D normalMap;

uniform int N;

#define LAMBDA 1

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);

	const float d00 = imageLoad(displacement, x).y;
	const float d10 = imageLoad(displacement, ivec2( mod(x + vec2(1, 0), vec2(N)) )).y;
	const float dn10 = imageLoad(displacement, ivec2( mod(x + vec2(-1, 0), vec2(N)) )).y;
	const float d01 = imageLoad(displacement, ivec2( mod(x + vec2(0, 1), vec2(N)) )).y;
	const float dn01 = imageLoad(displacement, ivec2( mod(x + vec2(0, -1), vec2(N)) )).y;

	const float dXdx = (dn10 - 2 * d00 + d10);
	const float dYdy = (dn01 - 2 * d00 + d01);

	imageStore(normalMap, x, vec4(dXdx, dYdy, d00, 1.0));
}

#endif

