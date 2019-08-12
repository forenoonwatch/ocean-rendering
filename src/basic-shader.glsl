
#if defined(VS_BUILD)

out vec3 localPos;
out vec3 lightDir;

layout(location = 0) in vec2 xyPos;
layout(location = 1) in vec4 adjacent;
layout(location = 2) in mat4 transforms[2];

layout (std140) uniform ShaderData {
	vec3 cameraPosition;
};

float height(vec2 pos) {
	return sin(pos.x);
}

vec3 getOceanPosition(vec2 pos) {
	vec4 a = transforms[1] * vec4(pos, 1.0, 1.0);
	vec4 b = transforms[1] * vec4(pos, -1.0, 1.0);
	a /= a.w;
	b /= b.w;

	const vec3 v = b.xyz - a.xyz;
	const float t = (0.0 - a.y) / v.y;
	
	vec3 posV3 = a.xyz + v * t;
	posV3.y = height(posV3.xz);

	return posV3;
}

void main() {
	vec3 position = getOceanPosition(xyPos);
	vec3 p1 = getOceanPosition(xyPos + adjacent.xy);
	vec3 p2 = getOceanPosition(xyPos + adjacent.zw);

	vec3 T = normalize(p1 - position);
	vec3 B = normalize(p2 - position);
	vec3 N = normalize(cross(T, B));
	mat3 TBN = mat3(T, cross(N, T), N);

	localPos = (position + vec3(cameraPosition.x, 0.0, cameraPosition.z)) * TBN;
	lightDir = normalize(vec3(0.0, -5.0, 0.0) - localPos) * TBN;

	gl_Position = transforms[0] * vec4(position, 1.0);
}

#elif defined(FS_BUILD)

in vec3 localPos;
in vec3 lightDir;

out vec4 outColor;

void main() {
	float diffuse = 0.9 * max(lightDir.z, 0.0) + 0.1;
	outColor = vec4(vec3(1.0, 1.0, 1.0) * diffuse, 1.0);
}

#endif
