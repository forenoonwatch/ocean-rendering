
#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;
layout (location = 2) in vec3 acceleration;
layout (location = 3) in vec4 transScale;
layout (location = 4) in vec2 timeToLive;

out vec3 position0;
out vec3 velocity0;
out vec3 acceleration0;

out vec4 transScale0;
out vec2 ttl0;

void main() {
	position0 = position;
	velocity0 = velocity;
	acceleration0 = acceleration;

	transScale0 = transScale;
	ttl0 = timeToLive;
}

#elif defined(GS_BUILD)

layout (points) in;
layout (points) out;

layout (max_vertices = 1) out;

in vec3 position0[];
in vec3 velocity0[];
in vec3 acceleration0[];

in vec4 transScale0[];
in vec2 ttl0[];

out vec3 position1;
out vec3 velocity1;
out vec3 acceleration1;

out vec4 transScale1;
out vec2 ttl1;

const float deltaTime = 1.0 / 60.0;

void main() {
	const float ttl = ttl0[0].x - deltaTime;

	if (ttl > 0.0) {
		const vec3 velocity = fma(acceleration0[0], vec3(deltaTime), velocity0[0]);

		position1 = fma(velocity, vec3(deltaTime), position0[0]);
		velocity1 = velocity;
		acceleration1 = acceleration0[0];

		transScale1 = transScale0[0];
		ttl1 = vec2(ttl, ttl0[0].y);

		EmitVertex();
		EndPrimitive();
	}
}

#elif defined(FS_BUILD)

void main() {}

#endif
