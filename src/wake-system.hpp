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
				const glm::vec2& driftVelocity,
				const glm::vec4& transScale,
				const glm::mat4& transform)
			: timeDriftData(timeToLive, timeToLive,
					driftVelocity.x, driftVelocity.y)
			, transScale(transScale) {
		transforms[0] = transform;	
	}

	glm::vec4 timeDriftData; // (timeToLive, initialTTL, driftX, driftZ)
	glm::vec4 transScale; // (trans0, trans1, scale0, scale1)
	glm::mat4 transforms[3]; // (model, mvp, invMVP)
};

class WakeSystem {
	public:
		WakeSystem(RenderContext& context, Texture& displacementMap,
				Sampler& displacementSampler, uintptr wakeBufferSize,
				uintptr inputBufferSize);

		void drawWake(const glm::vec2& driftVelocity,
				const glm::vec4& transScale,
				const glm::mat4& transform, float timeToLive);

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
