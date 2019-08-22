
#define OCEAN_SAMPLE 0.01
#define AMPLITUDE 2.0
#define F0 0.017 // F0 = (n1 - n2) / (n1 + n2); n1 = 1, n2 = 1.3

#include "bicubic-sampling.glh"

#define TEXEL_SIZE 256.0
#define SMOOTHNESS 2.0

#define GRID_LENGTH 256.0

const float texelSize = TEXEL_SIZE / SMOOTHNESS;

uniform sampler2D ocean;

layout (std140) uniform ShaderData {
	vec4 corners[4];
	vec3 cameraPosition;
	float time;
};

vec3 oceanData(vec2 pos) {
	const vec2 uv00 = floor(pos * texelSize) / texelSize;
	const vec2 frac = vec2(pos - uv00) * texelSize;

	vec3 height = textureBicubic(ocean, uv00, 1.0 / texelSize, frac);
	height = fma(height, vec3(2.0), vec3(-1.0));
	height.y *= AMPLITUDE;

	return height;
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	const vec3 data = oceanData(o.xz / o.w * OCEAN_SAMPLE);

	o.y = data.y * o.w;
	o.xz += data.xz * o.w;

	return o;
}

#if defined(VS_BUILD)

out vec3 lightDir;

out vec2 xyPos0;
out vec2 texCoord0;

out vec4 clipSpace;
out float fresnel;

layout (location = 0) in vec2 xyPos;
layout (location = 1) in mat4 transform;

vec2 getOceanTexCoord(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);

	return o.xz / o.w;
}

void main() {
	const vec4 p0Raw = getOceanPosition(xyPos);
	const vec3 p0 = p0Raw.xyz / p0Raw.w;

	const vec4 vertPos = transform * p0Raw;

	gl_Position = vertPos;
	texCoord0 = getOceanTexCoord(xyPos) * OCEAN_SAMPLE;
	xyPos0 = xyPos;
	clipSpace = vertPos;
	
	//lightDir = normalize(vec3(cos(0.1 * time), 0, sin(0.1 * time)));
	lightDir = normalize(vec3(1, 0, 1));
	
	//const float F = clamp(1.0 - dot(normal0, normalize(cameraPosition - p0)), 0.0, 1.0);
	
	fresnel = length(cameraPosition - p0);
	fresnel = clamp(exp(-pow(fresnel * 0.005, 1.5)), 0.0, 1.0);
}

#elif defined(FS_BUILD)

#define SPECULAR_STRENGTH 15.0
#define OPACITY 0.7
#define DISTORT_STRENGTH 0.03
#define AMBIENT_LIGHT 0.5

#define BRIGHT_THRESH vec3(0.2126, 0.7152, 0.0722)

//uniform sampler2D foldingMap;
//uniform sampler2D foam;

//uniform sampler2D normalMap;

uniform sampler2D reflectionMap;

//uniform sampler2D dudv;

in vec3 lightDir;

in vec2 xyPos0;
in vec2 texCoord0;

in vec4 clipSpace;
in float fresnel;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 brightColor;

//const vec3 oceanColor = vec3(0.416, 0.761, 0.906);
const vec3 oceanColor0 = vec3(11, 49, 58) / 255.0;
const vec3 oceanColor1 = vec3(35, 97, 93) / 255.0;

void main() {
	const vec2 ndc = (clipSpace.xy / clipSpace.w) * 0.5 + 0.5;

	const vec4 p0Raw = getOceanPosition(xyPos0);
	const vec4 p1Raw = getOceanPosition(xyPos0 + vec2(1.0 / (GRID_LENGTH - 1.0), 0.0));
	const vec4 p2Raw = getOceanPosition(xyPos0 + vec2(0.0, 1.0 / (GRID_LENGTH - 1.0)));

	const vec3 p0 = p0Raw.xyz / p0Raw.w;
	const vec3 p1 = p1Raw.xyz / p1Raw.w;
	const vec3 p2 = p2Raw.xyz / p2Raw.w;

	const vec3 normal = normalize(cross(p1 - p0, p2 - p0));

	const float diffuse = max(dot(lightDir, normal), 0.0);
	const float specular = SPECULAR_STRENGTH * pow(max(dot(normalize(cameraPosition - p0),
			reflect(-lightDir, normal)), 0.0), 64);

	const float light = AMBIENT_LIGHT + (1.0 - AMBIENT_LIGHT) * diffuse + specular;

	const float mask = 0;
	
	const float y = clamp(texture2D(ocean, texCoord0).y, 0.0, 1.0) * fresnel;
	const vec3 flect = texture2D(reflectionMap, vec2(ndc.x, -ndc.y)).rgb;
	const vec3 col = mix(oceanColor0, oceanColor1, y);

	const vec3 inColor = mix(mix(flect * col, col, fresnel), vec3(1), mask) * light;
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
