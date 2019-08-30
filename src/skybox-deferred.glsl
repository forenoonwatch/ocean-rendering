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

#define LOWER_LIMIT 0.0
#define UPPER_LIMIT 0.003

layout (std140) uniform LightingData {
	vec3 sunlightDir;
	float ambientLight;
	float specularStrength;
	float specularBlend;
	vec3 fogColor;
	float fogDensity;
	float fogGradient;
};

uniform samplerCube skybox;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;

void main() {
	const vec3 skyColor = texture(skybox, texCoord0).rgb;

	const float fact = clamp((texCoord0.y - LOWER_LIMIT)
			/ (UPPER_LIMIT - LOWER_LIMIT), 0.0, 1.0);

	outColor = vec4(mix(fogColor, skyColor, fact), 0.0);
	outPosition = vec4(0.0);
	outNormal = vec4(0.0);
}

#endif
