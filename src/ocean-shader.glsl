
#if defined(VS_BUILD)

out vec3 normal;

layout(location = 0) in vec2 xyPos;
layout(location = 1) in vec4 adjacent;
layout(location = 2) in mat4 transforms[2];

layout (std140) uniform ShaderData {
	vec4 corners[4];
};

float height(vec2 pos) {
	return sin(pos.x);
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	o.y = height(o.xz / o.w) * o.w;

	return o;
}

void main() {
	vec4 p0Raw = getOceanPosition(xyPos);
	vec4 p1Raw = getOceanPosition(xyPos + adjacent.xy);
	vec4 p2Raw = getOceanPosition(xyPos + adjacent.zw);

	vec3 p0 = p0Raw.xyz / p0Raw.w;
	vec3 p1 = p1Raw.xyz / p1Raw.w;
	vec3 p2 = p2Raw.xyz / p2Raw.w;

	normal = normalize(cross(p1 - p0, p2 - p0));

	gl_Position = transforms[0] * p0Raw;
}

#elif defined(FS_BUILD)

in vec3 normal;

out vec4 outColor;

void main() {
	float diffuse = normal.y;
	outColor = vec4(vec3(diffuse), 1.0);
}

#endif
