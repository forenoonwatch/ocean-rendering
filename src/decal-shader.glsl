#include "common.glh"

varying vec4 clipPosition;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 4) in mat4 transforms[2];

void main() {
	const vec4 vertPos = transforms[0] * vec4(position, 1.0);

	gl_Position = vertPos;
	clipPosition = vertPos;
}

#elif defined(FS_BUILD)

uniform mat4 invMVP;

uniform sampler2D depthBuffer;
uniform sampler2D normalBuffer;

uniform sampler2D diffuse;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const vec2 screenPosition = clipPosition.xy / clipPosition.w;
	
	const vec2 depthUV = fma(screenPosition, vec2(0.5), vec2(0.5));
	const float sceneDepth = fma(texture2D(depthBuffer, depthUV).r, 2.0, -1.0);
	const vec3 sceneNormal = texture2D(normalBuffer, depthUV).rgb;
	
	vec4 viewCoords = invMVP * vec4(screenPosition, sceneDepth, 1.0);
	viewCoords /= viewCoords.w;

	const vec2 texCoords = fma(viewCoords.xz, vec2(0.5), vec2(0.5));

	viewCoords = abs(viewCoords);

	if (viewCoords.x > 1.0 || viewCoords.y > 1.0 || viewCoords.z > 1.0) {
		discard;
	}
	else {
		const vec3 texNormal = fma(texture(normalMap, texCoords).rbg, vec3(2.0), vec3(-1.0));
		const float roughness = texture(roughnessMap, texCoords).r;
		const float ao = texture(aoMap, texCoords).r;

		const vec3 normal = normalize(sceneNormal + normalize(texNormal));

		outColor = texture(diffuse, texCoords);
		outNormal = vec4(normal, 1.0);
		outLighting = vec4(0.0, roughness, ao, 1.0);
	}
}

#endif
