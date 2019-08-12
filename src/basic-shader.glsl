
#if defined(VS_BUILD)

layout(location = 0) in vec4 position;
layout(location = 1) in mat4 transform;

layout (std140) uniform ShaderData {
	vec3 cameraPosition;
};

void main() {
	gl_Position = transform * position;
}

#elif defined(FS_BUILD)

out vec4 outColor;

void main() {
	outColor = vec4(1.0, 1.0, 1.0, 1.0);
}

#endif
