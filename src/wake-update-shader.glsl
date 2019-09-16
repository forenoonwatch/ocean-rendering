#include "scene-info.glh"
#include "ocean-common.glh"

#if defined(VS_BUILD)

layout (location = 0) in float timeToLive;
layout (location = 1) in mat4 transform;

out float ttl0;
out mat4 transform0;

void main() {
	ttl0 = timeToLive;
	transform0 = transform;
}

#elif defined(GS_BUILD)

layout (points) in;
layout (points) out;

layout (max_vertices = 1) out;

uniform sampler2D displacement;

in float ttl0[];
in mat4 transform0[];

out float ttl1;

out mat4 transform01;
out mat4 transform11;
out mat4 transform21;

const float deltaTime = 1.0 / 60.0;

float height(vec2 pos) {
	return texture2D(displacement, OCEAN_SAMPLE * pos).y
			* amplitude;
}

void main() {
	const float ttl = ttl0[0] - deltaTime;

	if (ttl > 0.0) {
		mat4 transform = transform0[0];

		vec3 pos = transform[3].xyz / transform[3][3];
		vec3 right = transform[0].xyz;
		vec3 forward = transform[2].xyz;

		vec3 ptFwd = pos + forward;
		vec3 ptBack = pos - forward;

		vec3 ptLeft = pos - right;
		vec3 ptRight = pos + right;

		ptFwd.y = height(ptFwd.xz);
		ptBack.y = height(ptBack.xz);
		
		ptLeft.y = height(ptLeft.xz);
		ptRight.y = height(ptRight.xz);

		right = normalize(ptRight - ptLeft);
		forward = normalize(ptFwd - ptBack);
		vec3 up = cross(forward, right);

		pos.y = 0.25 * (ptFwd.y + ptBack.y + ptLeft.y + ptRight.y);
		
		transform = mat4(vec4(right, 0.0), vec4(up, 0.0),
			vec4(forward, 0.0), vec4(pos, 1.0));

		mat4 mvp = viewProjection * transform;
		
		ttl1 = ttl;

		transform01 = transform;
		transform11 = mvp;
		transform21 = inverse(mvp);

		EmitVertex();
		EndPrimitive();
	}
}

#elif defined(FS_BUILD)

void main() {}

#endif
