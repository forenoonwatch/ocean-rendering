#include "common.glh"

#include "scene-info.glh"
#include "lighting.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

float distributionGGX(vec3 N, vec3 H, float roughness) {
	float a2 = roughness * roughness;
	a2 *= a2;

	float nDotH2 = max(dot(N, H), 0.0);
	nDotH2 *= nDotH2;

	return a2 / max(nDotH2 * (a2 - 1.0) + 1.0, 0.001);
}

float geometrySchlickGGX(float nDotV, float roughness) {
	float k = roughness + 1.0;
	k = (k * k) / 8.0;

	return nDotV / (nDotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	const float ggx1 = geometrySchlickGGX(max(dot(N, V), 0.0), roughness);
	const float ggx2 = geometrySchlickGGX(max(dot(N, L), 0.0), roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	cosTheta = 1.0 - cosTheta;

	float ct5 = cosTheta * cosTheta;
	ct5 = ct5 * ct5 * cosTheta;

	return F0 + (vec3(1.0) - F0) * ct5;
}

uniform sampler2D colorBuffer; // vec3 color, float lightPower
uniform sampler2D normLightBuffer; // vec2 normXY, float shininess, float reflectivity
uniform sampler2D depthBuffer; // float depth

//uniform samplerCube reflectionMap;

layout (location = 0) out vec4 outColor;
layout (location = 2) out vec4 brightColor;

void main() {
	const vec2 screenPosition = fma(gl_FragCoord.xy / displaySize, vec2(2.0), vec2(-1.0));
	const ivec2 texel = ivec2(gl_FragCoord.xy);

	const vec4 colorSpec = texelFetch(colorBuffer, texel, 0);
	const vec4 normLight = texelFetch(normLightBuffer, texel, 0);
	const float depth = fma(texelFetch(depthBuffer, texel, 0).x, 2.0, -1.0);

	const vec3 albedo = pow(colorSpec.xyz, vec3(2.2));

	const vec3 normal = vec3(normLight.xy, 1.0 - sqrt(normLight.x * normLight.x
			+ normLight.y * normLight.y));

	const vec4 rawPosition = invVP * vec4(screenPosition, depth, 1.0);
	const vec3 position = rawPosition.xyz / rawPosition.w;

	vec3 pointToEye = cameraPosition - position;
	const float cameraDist = length(pointToEye);
	pointToEye /= cameraDist;

	const vec3 F0 = mix(vec3(0.04), albedo, normLight.z);

	vec3 Lo = vec3(0.0);

	const vec3 H = normalize(pointToEye - sunlightDir);
	const vec3 radiance = vec3(1.0);

	const float NDF = distributionGGX(normal, H, normLight.w);
	const float G = geometrySmith(normal, pointToEye, -sunlightDir, normLight.w);
	const vec3 F = fresnelSchlick(clamp(dot(H, pointToEye), 0.0, 1.0), F0);

	const vec3 specular = (NDF * G * F) / max(4.0 * max(dot(normal, pointToEye), 0.0)
			* max(dot(normal, -sunlightDir), 0.0), 0.001);
	
	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - normLight.z;

	Lo += (kD * albedo / M_PI + specular)
			* radiance * max(dot(normal, -sunlightDir), 0.0);

	const vec3 ambient = vec3(0.03) * albedo;

	const float fogVisibility = clamp(exp(-pow(cameraDist * fogDensity, fogGradient)), 0.0, 1.0);

	//vec3 flect = texture(reflectionMap, reflect(-pointToEye, normal)).rgb * Lo;

	vec3 inColor = ambient + Lo;

	//vec3 inColor = mix(mix(colorSpec.xyz, ambient + Lo, colorSpec.w),
	//		flect, 1.0 - normLight.w);
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

