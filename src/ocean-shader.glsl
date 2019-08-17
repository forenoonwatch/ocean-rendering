
#if defined(VS_BUILD)

out vec3 normal;
out vec2 texCoord0;

uniform sampler2D ocean;

layout(location = 0) in vec2 xyPos;
layout(location = 1) in vec4 adjacent;
layout(location = 2) in mat4 transform;

layout (std140) uniform ShaderData {
	vec4 corners[4];
};

vec3 oceanData(vec2 pos) {
	return fma(texture2D(ocean, pos).xyz, vec3(2.0), vec3(-1.0))
			* vec3(1.0, 2.0, 1.0)
			+ 0.01 * texture2D(ocean, 100 * pos).xyz;
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	//o.y = height(o.xz / o.w * 0.01) * o.w;
	const vec3 data = oceanData(o.xz / o.w * 0.01);

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
	vec4 p0Raw = getOceanPosition(xyPos);
	vec4 p1Raw = getOceanPosition(xyPos + adjacent.xy);
	vec4 p2Raw = getOceanPosition(xyPos + adjacent.zw);

	vec3 p0 = p0Raw.xyz / p0Raw.w;
	vec3 p1 = p1Raw.xyz / p1Raw.w;
	vec3 p2 = p2Raw.xyz / p2Raw.w;

	normal = normalize(cross(p1 - p0, p2 - p0));

	gl_Position = transform * p0Raw;
	texCoord0 = getOceanTexCoord(xyPos) * 0.01;
}

#elif defined(FS_BUILD)

#define FOAM_THRESH 0.2

uniform sampler2D ocean;
uniform sampler2D foldingMap;
uniform sampler2D foam;

in vec3 normal;
in vec2 texCoord0;

out vec4 outColor;

const vec3 oceanColor = vec3(0, 0.41, 0.58);

void main() {
	const float diffuse = dot(normal, normalize(vec3(1, 1, 0)));
	const float mask = texture2D(foldingMap, texCoord0).y;
	const vec3 foamCol = texture2D(foam, 10.0 * texCoord0).rgb;	

	outColor = vec4(mix(oceanColor, foamCol, mask) * diffuse, 1.0);
	//outColor = vec4(vec3(diffuse), 1.0);
}

#endif
