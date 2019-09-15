
#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;
layout (location = 2) in float timeToLive;

out vec3 position0;
out vec3 velocity0;
out float ttl0;

void main() {
	position0 = position;
	velocity0 = velocity;
	ttl0 = timeToLive;
}

#elif defined(GS_BUILD)

layout (points) in;
layout (points) out;

layout (max_vertices = 1) out;

in vec3 position0[];
in vec3 velocity0[];
in float ttl0[];

out vec3 position1;
out vec3 velocity1;
out float ttl1;

const float deltaTime = 1.0 / 60.0;

void main() {
	const float ttl = ttl0[0] - deltaTime;

	if (ttl > 0.0) {
		const vec3 velocity = velocity0[0];

		position1 = fma(velocity, vec3(deltaTime), position0[0]);
		velocity1 = velocity;
		ttl1 = ttl;

		EmitVertex();
		EndPrimitive();
	}
}

#elif defined(FS_BUILD)

void main() {}

#endif
