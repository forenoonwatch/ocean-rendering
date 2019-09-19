#include "scene-info.glh"
#include "ocean-common.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec4 timeDriftData;
layout (location = 1) in vec4 transScale;
layout (location = 2) in mat4 transform;

out vec4 timeDriftData0;
out vec4 transScale0;
out mat4 transform0;

void main() {
	timeDriftData0 = timeDriftData;
	transScale0 = transScale;
	transform0 = transform;
}

#elif defined(GS_BUILD)

layout (points) in;
layout (points) out;

layout (max_vertices = 1) out;

uniform sampler2D displacement;

in vec4 timeDriftData0[];
in vec4 transScale0[];
in mat4 transform0[];

out vec4 timeDriftData1;
out vec4 transScale1;

out mat4 transform01;
out mat4 transform11;
out mat4 transform21;

const float deltaTime = 1.0 / 60.0;

float height(vec2 pos) {
	return texture2D(displacement, OCEAN_SAMPLE * pos).y
			* amplitude;
}

void main() {
	const float ttl = timeDriftData0[0].x - deltaTime;

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
		pos.xz = fma(timeDriftData0[0].zw, vec2(deltaTime), pos.xz);

		const float scale = mix(transScale0[0].z, transScale0[0].w,
				1.0 - ttl / timeDriftData0[0].y);
		
		transform = mat4(vec4(right * scale, 0.0), vec4(up, 0.0),
			vec4(forward * scale, 0.0), vec4(pos, 1.0));

		mat4 mvp = viewProjection * transform;
		
		timeDriftData1 = vec4(ttl, timeDriftData0[0].yzw);
		transScale1 = transScale0[0];

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
