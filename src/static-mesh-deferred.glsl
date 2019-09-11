#include "common.glh"

#include "normal-encoding.glh"

varying vec2 texCoord0;
varying mat3 TBN;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in mat4 transforms[2];

void main() {
	const vec4 vertPos = transforms[0] * vec4(position, 1.0);

	const vec3 N = normalize(vec3(transforms[1] * vec4(normal, 0.0)));
	vec3 T = normalize(vec3(transforms[1] * vec4(tangent, 0.0)));
	T = normalize(T - dot(T, N) * N);

	gl_Position = vertPos;
	
	texCoord0 = texCoord;
	TBN = mat3(T, cross(N, T), N);
}

#elif defined(FS_BUILD)

uniform sampler2D diffuse;
uniform sampler2D normalMap;

uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const float metallic = 0.0;
	const float roughness = texture(roughnessMap, texCoord0).r;
	const float ao = texture(aoMap, texCoord0).r;

	vec3 normal = fma(texture(normalMap, texCoord0).rgb, vec3(2.0), vec3(-1.0));
	normal = TBN * normalize(normal);

	outColor = vec4(texture(diffuse, texCoord0).rgb, 1.0);
	outNormal = vec4(normal, 1.0);
	outLighting = vec4(metallic, roughness, ao, 1.0);
}

#endif
