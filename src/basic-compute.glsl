
#if defined(CS_BUILD)

layout (local_size_x = 1, local_size_y = 1) in;

layout (std430, binding = 0) buffer myData {
	float v[4];
};

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);

	v[x.x] += 5.0;
}

#endif
