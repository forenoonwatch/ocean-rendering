#include "common.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) readonly uniform image2D displacement;
layout (binding = 1, rgba32f) writeonly uniform image2D foldingMap;

uniform int N;

#define LAMBDA 1

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);

	const vec3 d00 = imageLoad(displacement, x).xyz;
	const vec3 d10 = imageLoad(displacement, ivec2( mod(x + vec2(1, 0), vec2(N)) )).xyz;
	const vec3 dn10 = imageLoad(displacement, ivec2( mod(x + vec2(-1, 0), vec2(N)) )).xyz;
	const vec3 d01 = imageLoad(displacement, ivec2( mod(x + vec2(0, 1), vec2(N)) )).xyz;
	const vec3 dn01 = imageLoad(displacement, ivec2( mod(x + vec2(0, -1), vec2(N)) )).xyz;

	const float dXdx = (dn10.x - 2 * d00.x + d10.x);
	const float dZdz = (dn01.z - 2 * d00.z + d01.z);
	const float dXdz = (dn01.x - 2 * d00.x + d01.x);
	const float dZdx = (dn10.z - 2 * d00.z + d10.z);

	const float Jxx = 1 + LAMBDA * dXdx;
	const float Jxy = 1 + LAMBDA * dXdz;
	const float Jyx = 1 + LAMBDA * dZdx;
	const float Jyy = 1 + LAMBDA * dZdz;

	const float J = Jyy * Jxx - Jxy * Jyx;

	imageStore(foldingMap, x, vec4(vec3(J), 1.0));
}

#endif
