#pragma once

#include "render-context.hpp"

class Sampler {
	public:
		Sampler(RenderContext& context, uint32 minFilter = GL_NEAREST,
				uint32 magFilter = GL_NEAREST, uint32 wrapU = GL_CLAMP_TO_EDGE,
				uint32 wrapV = GL_CLAMP_TO_EDGE);

		inline uint32 getID() { return samplerID; }

		~Sampler();
	private:
		NULL_COPY_AND_ASSIGN(Sampler);

		RenderContext* context;

		uint32 samplerID;
};
