#include "common.glh"
#include "lighting.glh"

varying vec2 texCoord0;

varying vec3 pointToEye;
varying vec3 lightDir;
varying float fogVisibility;

layout (std140, binding = 0) uniform OceanData {
	vec4 corners[4];
	vec3 cameraPosition;
	float amplitude;
	float detailAmplitude;
	float lambda;
};

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in mat4 transforms[2];

void main() {
	const vec4 vertPos = transforms[0] * vec4(position, 1.0);
	const vec4 p0 = transforms[1] * vec4(position, 1.0);

	const vec3 N = normalize((transforms[1] * vec4(normal, 0.0)).xyz);

	vec3 T = (transforms[1] * vec4(tangent, 0.0)).xyz;
	T = normalize(T - dot(T, N) * N);

	const mat3 TBN = mat3(T, cross(N, T), N);

	const vec3 cameraDir = cameraPosition - (p0.xyz / p0.w);

	pointToEye = normalize(cameraDir) * TBN;
	lightDir = sunlightDir * TBN;

	gl_Position = vertPos;
	texCoord0 = texCoord;

	fogVisibility = clamp(exp(-pow(length(cameraDir) * fogDensity, fogGradient)), 0.0, 1.0);
}

#elif defined(FS_BUILD)

layout (location = 0) out vec4 outColor;

void main() {
	const vec3 normal = vec3(0, 0, 1);

	const float diffuse = max(dot(normal, lightDir), 0.0);
	const float specular = specularStrength * pow(max(dot(pointToEye,
			reflect(-lightDir, normal)), 0.0), specularBlend);

	const float light = ambientLight + (1.0 - ambientLight) * diffuse
			+ specular;

	outColor = vec4(mix(fogColor, vec3(light), fogVisibility), 1.0);
}

#endif
