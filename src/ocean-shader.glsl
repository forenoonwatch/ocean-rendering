
#define OCEAN_SAMPLE 0.01

#if defined(VS_BUILD)

#include "bicubic-sampling.glh"

#define TEXEL_SIZE 256.0
#define SMOOTHNESS 2.0

const float texelSize = TEXEL_SIZE / SMOOTHNESS;

out vec3 localPos;
out vec3 lightDir;
out vec2 texCoord0;

uniform sampler2D ocean;

layout (location = 0) in vec2 xyPos;
layout (location = 1) in vec4 adjacent;
layout (location = 2) in mat4 transform;

layout (std140) uniform ShaderData {
	vec4 corners[4];
	vec3 cameraPosition;
};

vec3 oceanData(vec2 pos) {
	const vec2 uv00 = floor(pos * texelSize) / texelSize;
	const vec2 frac = vec2(pos - uv00) * texelSize;

	vec3 height = textureBicubic(ocean, uv00, 1.0 / texelSize, frac);
	height = fma(height, vec3(2.0), vec3(-1.0));
	height.y *= 2;

	return height;
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	//o.y = height(o.xz / o.w * 0.01) * o.w;
	const vec3 data = oceanData(o.xz / o.w * OCEAN_SAMPLE);

	o.y = data.y * o.w;
	o.xz += data.xz * o.w;

	return o;
}

vec2 getOceanTexCoord(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);

	return o.xz / o.w;
}

void main() {
	const vec4 p0Raw = getOceanPosition(xyPos);
	const vec4 p1Raw = getOceanPosition(xyPos + adjacent.xy);
	const vec4 p2Raw = getOceanPosition(xyPos + adjacent.zw);

	const vec3 p0 = p0Raw.xyz / p0Raw.w;
	const vec3 p1 = p1Raw.xyz / p1Raw.w;
	const vec3 p2 = p2Raw.xyz / p2Raw.w;

	const vec3 T = normalize(p1 - p0);
	const vec3 N = normalize(cross(T, p2 - p0));
	const vec3 B = cross(N, T);
	const mat3 TBN = mat3(T, B, N);

	gl_Position = transform * p0Raw;
	texCoord0 = getOceanTexCoord(xyPos) * OCEAN_SAMPLE;

	localPos = p0;
	lightDir = normalize(vec3(0, 10, 0) - localPos) * TBN;
}

#elif defined(FS_BUILD)

#define FOAM_THRESH 0.2
#define SPECULAR_STRENGTH 1

uniform sampler2D ocean;
uniform sampler2D foldingMap;
uniform sampler2D foam;

in vec3 lightDir;
in vec3 localPos;

in vec2 texCoord0;

out vec4 outColor;

layout (std140) uniform ShaderData {
	vec4 corners[4];
	vec3 cameraPosition;
};

const vec3 oceanColor = vec3(0, 0.41, 0.58);

void main() {
	//const float diffuse = dot(normal, normalize(vec3(1, 1, 0)));
	const float diffuse = max(lightDir.z, 0.0);
	const float specular = SPECULAR_STRENGTH * pow(max(dot(normalize(cameraPosition - localPos),
			reflect(-lightDir, vec3(0, 0, 1))), 0.0), 32);

	const float mask = texture2D(foldingMap, texCoord0).y;
	const vec3 foamCol = texture2D(foam, 10.0 * texCoord0).rgb;	

	outColor = vec4(mix(oceanColor, foamCol, mask) * (diffuse + specular), 1.0);
	//outColor = vec4(vec3(diffuse), 1.0);
}

#endif
