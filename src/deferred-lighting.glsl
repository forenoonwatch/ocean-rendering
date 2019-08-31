#include "common.glh"

#include "scene-info.glh"
#include "lighting.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

uniform sampler2D colorBuffer; // vec3 color, float lightPower
uniform sampler2D normLightBuffer; // vec2 normXY, float shininess, float reflectivity
uniform sampler2D depthBuffer; // float depth

uniform samplerCube reflectionMap;

layout (location = 0) out vec4 outColor;
layout (location = 2) out vec4 brightColor;

void main() {
	const vec2 screenPosition = fma(gl_FragCoord.xy / displaySize, vec2(2.0), vec2(-1.0));
	const ivec2 texel = ivec2(gl_FragCoord.xy);

	const vec4 colorSpec = texelFetch(colorBuffer, texel, 0);
	const vec4 normLight = texelFetch(normLightBuffer, texel, 0);
	const float depth = fma(texelFetch(depthBuffer, texel, 0).x, 2.0, -1.0);

	const vec3 normal = vec3(normLight.xy, 1.0 - sqrt(normLight.x * normLight.x
			+ normLight.y * normLight.y));

	const vec4 rawPosition = invVP * vec4(screenPosition, depth, 1.0);
	const vec3 position = rawPosition.xyz / rawPosition.w;

	vec3 pointToEye = cameraPosition - position;
	const float cameraDist = length(pointToEye);
	pointToEye /= cameraDist;

	const float diffuse = max(dot(sunlightDir, normal), 0.0);
	const float specular = specularStrength
			* pow(max(dot(pointToEye, reflect(-sunlightDir, normal)), 0.0),
			specularBlend) * normLight.z;

	const float light = ambientLight + (1.0 - ambientLight) * diffuse + specular;
	const float fogVisibility = clamp(exp(-pow(cameraDist * fogDensity, fogGradient)), 0.0, 1.0);

	vec3 flect = texture(reflectionMap, reflect(-pointToEye, normal)).rgb * light;

	vec3 inColor = mix(mix(colorSpec.xyz, colorSpec.xyz * light, colorSpec.w),
			flect, normLight.w);
	inColor = mix(fogColor, inColor, fogVisibility);

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

