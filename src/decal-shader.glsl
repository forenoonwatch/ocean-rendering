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
uniform sampler2D testTexture;

layout (location = 0) out vec4 outColor;

void main() {
	const vec2 screenPosition = clipPosition.xy / clipPosition.w;
	
	const vec2 depthUV = fma(screenPosition, vec2(0.5), vec2(0.5));
	const float sceneDepth = fma(texture2D(depthBuffer, depthUV).r, 2.0, -1.0);
	
	vec4 viewCoords = invMVP * vec4(screenPosition, sceneDepth, 1.0);
	viewCoords /= viewCoords.w;

	const vec2 texCoords = fma(viewCoords.xz, vec2(0.5), vec2(0.5));

	viewCoords = abs(viewCoords);

	if (viewCoords.x > 1.0 || viewCoords.y > 1.0 || viewCoords.z > 1.0) {
		discard;
	}
	else {
		outColor = texture2D(testTexture, texCoords);
	}
}

#endif
