
#if defined(CS_BUILD)

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform writeonly image2D img_output;

void main() {
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	const vec3 color = vec3(gl_GlobalInvocationID) / vec3(gl_NumWorkGroups);
	const vec4 pixel = vec4(color, 1.0);

	imageStore(img_output, pixelCoords, pixel);
}

#endif
