
#if defined(VS_BUILD)

out vec3 texCoord0;

layout (location = 0) in vec3 position;
layout (location = 1) in mat4 transform;

void main() {
	const vec4 pos = transform * vec4(position, 1.0);
	
	gl_Position = pos.xyww;
	texCoord0 = position;
}

#elif defined(FS_BUILD)

uniform samplerCube skybox;

in vec3 texCoord0;

out vec4 outColor;

void main() {
	outColor = texture(skybox, texCoord0);
}

#endif
