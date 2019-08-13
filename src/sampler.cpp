#include "sampler.hpp"

Sampler::Sampler(RenderContext& context, uint32 minFilter,
			uint32 magFilter, uint32 wrapU, uint32 wrapV)
		: context(&context)
		, samplerID(-1) {
	glGenSamplers(1, &samplerID);

	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_S, wrapU);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_T, wrapV);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_R, wrapV);
	glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, magFilter);
	glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, minFilter);
}

Sampler::~Sampler() {
	glDeleteSamplers(1, &samplerID);
}
