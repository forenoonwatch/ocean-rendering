#include "common.glh"

#include "normal-encoding.glh"

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

uniform sampler2D diffuse;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormLight;

void main() {
	const float metallic = 0.0;
	const float roughness = 1.0;

	outColor = vec4(texture(diffuse, texCoord0).rgb, metallic);
	//outNormLight = vec4(normal0, roughness);
	outNormLight = vec4(encodeNormal(normal0), 1.0, roughness);
}

#endif
