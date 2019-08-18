#include "common.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) readonly uniform image2D displacement;
layout (binding = 1, rgba32f) writeonly uniform image2D jacobian;

uniform int N;

#define LAMBDA 1.2

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);

	const vec3 d00 = imageLoad(displacement, x).xyz;
	const vec3 d10 = imageLoad(displacement, ivec2( mod(x + vec2(1, 0), vec2(N)) )).xyz;
	const vec3 d01 = imageLoad(displacement, ivec2( mod(x + vec2(0, 1), vec2(N)) )).xyz;

	const float Jxx = 1 + LAMBDA * (d10.x - d00.x);
	const float Jxy = 1 + LAMBDA * (d01.x - d00.x);
	const float Jyy = 1 + LAMBDA * (d01.z - d00.z);

	//const float Jxx = 1 + LAMBDA;
	//const float Jxy = 1 + LAMBDA * d00.x;
	//const float Jyx = 1 + LAMBDA * d00.z;
	//const float Jyy = 1 + LAMBDA;

	//const float dy = (d10.y - d00.y) * (d01.y - d00.y) * N;

	const float J = clamp(Jyy * Jxx - Jxy * Jxy, 0.0, 1.0);
	//float J = (Jyy * Jxx - Jxy * Jyx);//-100.0 * (Jxx * Jyy - Jxy * Jyx);
	//float J = (LAMBDA + (d10.x - d00.x) + (d01.z - d00.z)) * (d00.y);
	//J = clamp(J, 0.0, 0.6);

	imageStore(jacobian, ivec2(gl_GlobalInvocationID.xy), vec4(vec3(J), 1.0));
}

#endif
