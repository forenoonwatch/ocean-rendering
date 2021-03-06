#pragma once

#include <engine/rendering/shader.hpp>
#include <engine/rendering/transform-feedback.hpp>
#include <engine/rendering/input-stream-buffer.hpp>

#include <engine/rendering/render-target.hpp>

#include <vector>

#include <GLM/glm.hpp>

struct Particle {
	inline Particle(const glm::vec3& position,
				const glm::vec3& velocity, const glm::vec3& acceleration,
				const glm::vec4& transScale, float timeToLive)
			: position(position)
			, velocity(velocity)
			, acceleration(acceleration)
			, transScale(transScale)
			, timeData(timeToLive, timeToLive) {}

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec4 transScale;
	glm::vec2 timeData;
};

class ParticleSystem {
	public:
		ParticleSystem(RenderContext& context,
				uintptr particleBufferSize, uintptr inputBufferSize);

		void drawParticle(const Particle& particle);
		void drawParticle(const glm::vec3& position, const glm::vec3& velocity,
				const glm::vec3& acceleration, const glm::vec4& transScale,
				float timeToLive);

		void update();
		void draw(RenderTarget& target, Texture& texture, Sampler& sampler);

		~ParticleSystem();
	private:
		NULL_COPY_AND_ASSIGN(ParticleSystem);

		RenderContext* context;

		TransformFeedback* feedback;
		InputStreamBuffer* inputBuffer;

		Shader* transformShader;
		Shader* billboardShader;

		std::vector<Particle> particleBuffer;
		uint32 lastNumParticles;
};
