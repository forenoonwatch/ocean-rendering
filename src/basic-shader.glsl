
#if defined(VS_BUILD)

out vec2 texCoord0;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 offset;

layout (std140) uniform ShaderData {
	vec3 cameraPosition;
};

void main() {
	gl_Position = vec4(position + offset, 0.0, 1.0);
	texCoord0 = position;
}

#elif defined(FS_BUILD)

in vec2 texCoord0;

uniform sampler2D diffuse;

out vec4 outColor;

void main() {
	outColor = texture2D(diffuse, texCoord0);
}

#endif
