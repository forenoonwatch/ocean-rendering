#include "common.glh"

#include "scene-info.glh"
#include "lighting.glh"

varying vec2 texCoord0;
varying vec3 normal0;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in mat4 transforms[2];

void main() {
	const vec4 vertPos = transforms[0] * vec4(position, 1.0);

	gl_Position = vertPos;

	texCoord0 = texCoord;
	normal0 = normalize((transforms[1] * vec4(normal, 0.0)).xyz);
}

#elif defined(FS_BUILD)

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;

void main() {
	outColor = vec4(1.0);
	outNormal = vec4(normal0, 1.0);
}

#endif
