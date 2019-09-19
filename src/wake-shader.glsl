#include "common.glh"

varying vec4 clipPosition;
varying mat4 invMVP;
varying float transparency;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 timeDriftData;
layout (location = 2) in vec4 transScale;
layout (location = 3) in mat4 transforms[3];

void main() {
	const vec4 vertPos = transforms[1] * vec4(position, 1.0);

	gl_Position = vertPos;
	clipPosition = vertPos;
	invMVP = transforms[2];

	transparency = clamp(mix(transScale.x, transScale.y,
			1.0 - timeDriftData.x / timeDriftData.y), 0.0, 1.0);
}

#elif defined(FS_BUILD)

uniform sampler2D depthBuffer;
//uniform sampler2D colorBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D lightingBuffer;

uniform sampler2D diffuse;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const vec2 screenPosition = clipPosition.xy / clipPosition.w;
	
	const vec2 sceneUV = fma(screenPosition, vec2(0.5), vec2(0.5));

	const float sceneDepth = fma(texture2D(depthBuffer, sceneUV).r, 2.0, -1.0);
	//const vec3 sceneColor = texture2D(colorBuffer, sceneUV).rgb;
	const vec3 sceneNormal = texture2D(normalBuffer, sceneUV).rgb;
	const vec4 sceneLighting = texture2D(lightingBuffer, sceneUV);
	
	vec4 viewCoords = invMVP * vec4(screenPosition, sceneDepth, 1.0);
	viewCoords /= viewCoords.w;

	const vec2 texCoords = fma(viewCoords.xz, vec2(0.5), vec2(0.5));

	viewCoords = abs(viewCoords);

	if (viewCoords.x >= 1.0 || viewCoords.y >= 1.0 || viewCoords.z >= 1.0) {
		discard;
	}
	else {
		const vec4 foam = texture2D(diffuse, texCoords);

		outColor = vec4(foam.xyz, foam.w * transparency);
		//outColor = vec4(mix(sceneColor, vec3(1.0), foamMask), 1.0);
		outNormal = vec4(sceneNormal, 1.0);
		outLighting = mix(sceneLighting, vec4(0.1, 0.8, 1.0, 1.0),
				foam.x * transparency);
	}
}

#endif
