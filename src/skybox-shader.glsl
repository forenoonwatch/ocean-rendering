#include "common.glh"

varying vec3 texCoord0;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in mat4 transform;

void main() {
	const vec4 pos = transform * vec4(position, 1.0);
	
	gl_Position = pos.xyww;
	texCoord0 = position;
}

#elif defined(FS_BUILD)

uniform samplerCube skybox;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 brightColor;

void main() {
	outColor = texture(skybox, texCoord0);
	brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

#endif
