#include "common.glh"

#include "scene-info.glh"
#include "lighting.glh"

varying vec2 texCoord0;

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
	texCoord0 = texCoord;
}

#elif defined(FS_BUILD)

uniform sampler2D colorBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D depthBuffer;

layout (location = 0) out vec4 outColor;
layout (location = 2) out vec4 brightColor;

void main() {
	const vec2 screenPosition = fma(texCoord0, vec2(2.0), vec2(-1.0));

	const vec4 colorSpec = texture2D(colorBuffer, texCoord0);

	const vec3 normal = texture2D(normalBuffer, texCoord0).xyz;
	const float depth = fma(texture2D(depthBuffer, texCoord0).x, 2.0, -1.0);

	const vec4 rawPosition = invVP * vec4(screenPosition, depth, 1.0);
	const vec3 position = rawPosition.xyz / rawPosition.w;

	vec3 pointToEye = cameraPosition - position;
	const float cameraDist = length(pointToEye);
	pointToEye /= cameraDist;

	const float diffuse = max(dot(sunlightDir, normal), 0.0);
	const float specular = specularStrength
			* pow(max(dot(pointToEye, reflect(-sunlightDir, normal)), 0.0),
			specularBlend) * colorSpec.w;

	const float light = ambientLight + (1.0 - ambientLight) * diffuse + specular;
	const float fogVisibility = clamp(exp(-pow(cameraDist * fogDensity, fogGradient)), 0.0, 1.0);

	const vec3 inColor = mix(fogColor, colorSpec.xyz * light, fogVisibility);
	const float brightness = dot(inColor, BRIGHT_THRESH);

	outColor = vec4(inColor, 1.0);

	if (brightness > 1.0) {
		brightColor = vec4(inColor, 1.0);
	}
	else {
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

#endif

