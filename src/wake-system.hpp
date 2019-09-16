#pragma once

#include "shader.hpp"

#include "vertex-array.hpp"
#include "transform-feedback.hpp"
#include "input-stream-buffer.hpp"

#include "render-query.hpp"

#include <vector>

#include <GLM/glm.hpp>

class DeferredRenderTarget;

struct WakeInstance {
	inline WakeInstance(float timeToLive,
				const glm::mat4& transform)
			: timeToLive(timeToLive) {
		transforms[0] = transform;	
	}

	float timeToLive;
	glm::mat4 transforms[3];
};

class WakeSystem {
	public:
		WakeSystem(RenderContext& context, Texture& displacementMap,
				Sampler& displacementSampler, uintptr wakeBufferSize,
				uintptr inputBufferSize);

		void drawWake(const glm::mat4& transform);

		void update();
		void draw(DeferredRenderTarget& target, Texture& texture,
				Sampler& sampler);

		~WakeSystem();
	private:
		NULL_COPY_AND_ASSIGN(WakeSystem);

		RenderContext* context;

		TransformFeedback* feedback;
		InputStreamBuffer* inputBuffer;

		Shader* transformShader;
		Shader* wakeShader;

		VertexArray* cubes[2];

		RenderQuery feedbackQuery;

		std::vector<WakeInstance> wakeBuffer;

		Texture* displacementMap;
		Sampler* displacementSampler;
};
