#include "common.glh"
#include "bicubic-sampling.glh"

#define OCEAN_SAMPLE 0.01
#define F0 0.017 // F0 = (n1 - n2) / (n1 + n2); n1 = 1, n2 = 1.3
#define SSS_POWER 2.0

#define TEXEL_SIZE 256.0
#define SMOOTHNESS 2.0

#define GRID_LENGTH 256.0

const float texelSize = TEXEL_SIZE / SMOOTHNESS;

uniform sampler2D ocean;

layout (std140) uniform OceanData {
	vec4 corners[4];
	vec3 cameraPosition;
	float amplitude;
};

layout (std140) uniform LightingData {
	vec3 sunlightDir;
	float ambientLight;
	float specularStrength;
	float specularBlend;
};

vec3 oceanData(vec2 pos) {
	const vec2 uv00 = floor(pos * texelSize) / texelSize;
	const vec2 frac = vec2(pos - uv00) * texelSize;

	vec3 height = textureBicubic(ocean, uv00, 1.0 / texelSize, frac);
	height.y *= amplitude;

	return height;
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	const vec3 data = oceanData(o.xz / o.w * OCEAN_SAMPLE);

	o.xyz += data * o.w;

	return o;
}

varying vec2 xyPos0;

varying vec4 clipSpace;
varying float fresnel;

#if defined(VS_BUILD)

layout (location = 0) in vec2 xyPos;
layout (location = 1) in mat4 transform;

void main() {
	const vec4 p0Raw = getOceanPosition(xyPos);
	const vec4 p1Raw = getOceanPosition(xyPos + vec2(1.0 / (GRID_LENGTH - 1.0), 0.0));
	const vec4 p2Raw = getOceanPosition(xyPos + vec2(0.0, 1.0 / (GRID_LENGTH - 1.0)));

	const vec3 p0 = p0Raw.xyz / p0Raw.w;
	const vec3 p1 = p1Raw.xyz / p1Raw.w;
	const vec3 p2 = p2Raw.xyz / p2Raw.w;

	const vec3 normal = normalize(cross(p2 - p0, p1 - p0));
	const vec4 vertPos = transform * p0Raw;

	gl_Position = vertPos;
	xyPos0 = xyPos;
	clipSpace = vertPos;
	
	const float F = clamp(1.0 - dot(normal, normalize(cameraPosition - p0)), 0.0, 1.0);
	fresnel = F * F;
	fresnel = F0 + (1.0 - F0) * (fresnel * fresnel * F);
}

#elif defined(FS_BUILD)

const vec3 oceanColor0 = vec3(31, 71, 87) / 255.0;
const vec3 oceanColor1 = vec3(18, 125, 120) / 255.0;

//uniform sampler2D reflectionMap;
uniform samplerCube reflectionMap;

//uniform sampler2D foldingMap;
//uniform sampler2D foam;

//uniform sampler2D dudv;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 brightColor;

void main() {
	const vec2 ndc = (clipSpace.xy / clipSpace.w) * 0.5 + 0.5;

	const vec4 p0Raw = getOceanPosition(xyPos0);
	const vec4 p1Raw = getOceanPosition(xyPos0 + vec2(1.0 / (GRID_LENGTH - 1.0), 0.0));
	const vec4 p2Raw = getOceanPosition(xyPos0 + vec2(0.0, 1.0 / (GRID_LENGTH - 1.0)));

	const vec3 p0 = p0Raw.xyz / p0Raw.w;
	const vec3 p1 = p1Raw.xyz / p1Raw.w;
	const vec3 p2 = p2Raw.xyz / p2Raw.w;

	const vec3 normal = normalize(cross(p2 - p0, p1 - p0));
	const vec3 pointToEye = normalize(cameraPosition - p0);

	const float diffuse = max(dot(sunlightDir, normal), 0.0);
	const float specular = specularStrength
			* pow(max(dot(pointToEye, reflect(-sunlightDir, normal)), 0.0), specularBlend);

	const float light = ambientLight + (1.0 - ambientLight) * diffuse + specular;
	
	const float sssFactor = clamp(SSS_POWER * (1.0 - normal.y), 0.0, 1.0)
			* max(sunlightDir.y, 0.0);
	//const vec3 flect = texture2D(reflectionMap, vec2(ndc.x, -ndc.y)).rgb * light;
	const vec3 flect = texture(reflectionMap, reflect(-pointToEye, normal)).rgb * light;

	const vec3 inColor = mix(mix(oceanColor0 * light, oceanColor1, sssFactor), flect, fresnel);
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
