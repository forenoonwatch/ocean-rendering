#include "common.glh"
#include "scene-info.glh"
#include "lighting.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;
layout (location = 2) in vec3 acceleration;
layout (location = 3) in vec4 transScale;
layout (location = 4) in vec2 timeToLive;

out vec2 transScale0;

void main() {
	const float timeScale = 1.0 - timeToLive.x / timeToLive.y;

	gl_Position = vec4(position, 1.0);
	transScale0 = mix(transScale.xz, transScale.yw, timeScale);
	transScale0.x = clamp(transScale0.x, 0.0, 1.0);
}

#elif defined(GS_BUILD)

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in vec2 transScale0[];

out vec3 normal1;
out vec2 texCoord1;
out float transparency1;

void main() {
	vec3 pos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(cameraPosition - pos);
	vec3 right = normalize(cross(toCamera, vec3(0.0, 1.0, 0.0)));
	vec3 up = cross(right, toCamera) * transScale0[0].y;
	right *= transScale0[0].y;

	normal1 = toCamera;
	transparency1 = transScale0[0].x;

	pos -= right;
	pos -= up;
	gl_Position = viewProjection * vec4(pos, 1.0);
	texCoord1 = vec2(0.0, 0.0);
	EmitVertex();

	pos = fma(up, vec3(2.0), pos);
	gl_Position = viewProjection * vec4(pos, 1.0);
	texCoord1 = vec2(0.0, 1.0);
	EmitVertex();

	pos = fma(up, vec3(-2.0), pos);
	pos = fma(right, vec3(2.0), pos);
	gl_Position = viewProjection * vec4(pos, 1.0);
	texCoord1 = vec2(1.0, 0.0);
	EmitVertex();

	pos = fma(up, vec3(2.0), pos);
	gl_Position = viewProjection * vec4(pos, 1.0);
	texCoord1 = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

#elif defined(FS_BUILD)

in vec3 normal1;
in vec2 texCoord1;
in float transparency1;

uniform sampler2D billboard;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const vec4 albedo = texture2D(billboard, texCoord1);
	const float diffuse = abs(dot(normal1, -sunlightDir));

	const vec3 color = albedo.xyz * mix(diffuse, 1.0, ambientLight);

	outColor = vec4(color, albedo.w * transparency1);
	outNormal = vec4(normal1, albedo.w * transparency1);
	outLighting = vec4(0.0, 1.0, 0.0, 1.0);
}

#endif
