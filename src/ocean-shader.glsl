
#define OCEAN_SAMPLE 0.01
#define AMPLITUDE 2.0
#define F0 0.017 // F0 = (n1 - n2) / (n1 + n2); n1 = 1, n2 = 1.3

#include "bicubic-sampling.glh"

#define TEXEL_SIZE 256.0
#define SMOOTHNESS 2.0

const float texelSize = TEXEL_SIZE / SMOOTHNESS;

uniform sampler2D ocean;

vec3 oceanData(vec2 pos) {
	const vec2 uv00 = floor(pos * texelSize) / texelSize;
	const vec2 frac = vec2(pos - uv00) * texelSize;

	vec3 height = textureBicubic(ocean, uv00, 1.0 / texelSize, frac);
	height = fma(height, vec3(2.0), vec3(-1.0));
	height.y *= AMPLITUDE;

	return height;
}

#if defined(VS_BUILD)

out vec3 localPos;
out vec3 lightDir;

out vec2 texCoord0;
out vec4 clipSpace;
out float fresnel;

layout (location = 0) in vec2 xyPos;
layout (location = 1) in vec4 adjacent;
layout (location = 2) in mat4 transform;

layout (std140) uniform ShaderData {
	vec4 corners[4];
	vec3 cameraPosition;
	float time;
};

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

	const vec4 vertPos = transform * p0Raw;

	gl_Position = vertPos;
	texCoord0 = getOceanTexCoord(xyPos) * OCEAN_SAMPLE;
	clipSpace = vertPos;

	localPos = p0;
	//lightDir = normalize(vec3(0, 0, 1)) * TBN;
	lightDir = normalize(vec3(1, -3, 1)) * TBN;
	
	const float F = clamp(1.0 - dot(N, normalize(cameraPosition - p0)), 0.0, 1.0);
	//fresnel = F;
	//fresnel = F * F;
	//fresnel = fresnel * fresnel * F;
	//fresnel = F0 + (1.0 - F0) * fresnel;
	
	fresnel = length(cameraPosition - p0);
	fresnel = clamp(exp(-pow(fresnel * 0.005, 1.5)), 0.0, 1.0);
}

#elif defined(FS_BUILD)

#define SPECULAR_STRENGTH 5.0
#define OPACITY 0.7
#define DISTORT_STRENGTH 0.03
#define AMBIENT_LIGHT 0.5

#define BRIGHT_THRESH vec3(0.2126, 0.7152, 0.0722)

//uniform sampler2D foldingMap;
//uniform sampler2D foam;

uniform sampler2D normalMap;

uniform sampler2D reflectionMap;

//uniform sampler2D dudv;

in vec3 lightDir;
in vec3 localPos;

in vec2 texCoord0;
in vec4 clipSpace;
in float fresnel;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 brightColor;

layout (std140) uniform ShaderData {
	vec4 corners[4];
	vec3 cameraPosition;
	float time;
};

//const vec3 oceanColor = vec3(0.416, 0.761, 0.906);
const vec3 oceanColor0 = vec3(11, 49, 58) / 255.0;
const vec3 oceanColor1 = vec3(35, 97, 93) / 255.0;

void main() {
	const vec2 ndc = (clipSpace.xy / clipSpace.w) * 0.5 + 0.5;
	
	const vec2 uv00 = floor(texCoord0 * texelSize) / texelSize;
	const vec2 frac = vec2(texCoord0 - uv00) * texelSize;

	vec3 od = textureBicubic(ocean, uv00, 1.0 / texelSize, frac);
	vec3 lp = od + vec3(texCoord0.x, 0, texCoord0.y) * 100.0;

	vec3 normal = textureBicubic(normalMap, uv00, 1.0 / texelSize, frac);
	normal = normalize(fma(normal, vec3(2.0), vec3(-1.0)));

	//const float diffuse = dot(normal, normalize(vec3(1, 1, 0)));
	const float diffuse = max(dot(lightDir, normal), 0.0);//max(lightDir.z, 0.0);
	const float specular = SPECULAR_STRENGTH * pow(max(dot(normalize(cameraPosition - lp),
			reflect(-lightDir, normal)), 0.0), 32);

	const float light = AMBIENT_LIGHT + (1.0 - AMBIENT_LIGHT) * diffuse + specular;

	/*const float mask = clamp(texture2D(foldingMap, texCoord0).y * texture2D(ocean, texCoord0).y,
			0.0, 1.0);
	const vec3 foamCol = texture2D(foam, 10.0 * texCoord0).rgb;

	vec2 distort = texture2D(dudv, 0.1 * texCoord0).xy;
	distort = fma(distort, vec2(2.0), vec2(-1.0)) * DISTORT_STRENGTH;
	distort.x *= 0.5 * sin(2 * time);

	const vec3 flect = texture2D(reflectionMap, vec2(ndc.x, -ndc.y) + distort).rgb;

	const vec3 col = mix(oceanColor, mix(flect, oceanColor, OPACITY), fresnel);

	outColor = vec4(mix(col, foamCol, mask) * light, 1.0);
	//outColor = vec4(vec3(diffuse), 1.0);*/
	
	const float mask = 0;//texture2D(foldingMap, texCoord0).x;
	
	float y = clamp(texture2D(ocean, texCoord0).y, 0.0, 1.0) * fresnel;
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
