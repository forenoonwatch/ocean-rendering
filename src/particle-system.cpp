#include "particle-system.hpp"

#include "util.hpp"

ParticleSystem::ParticleSystem(RenderContext& context,
			uintptr particleBufferSize, uintptr inputBufferSize)
		: context(&context) {
	const uint32 elementSizes[] = {3, 3, 1};
	const char* varyings[] = {"position1", "velocity1", "ttl1"};

	feedback = new TransformFeedback(context, ARRAY_SIZE_IN_ELEMENTS(elementSizes),
			elementSizes, particleBufferSize);
	inputBuffer = new InputStreamBuffer(context, ARRAY_SIZE_IN_ELEMENTS(elementSizes),
			elementSizes, inputBufferSize);

	std::stringstream ss;
	Util::resolveFileLinking(ss, "./src/particle-update-shader.glsl", "#include");
	transformShader = new Shader(context, ss.str(), varyings,
			ARRAY_SIZE_IN_ELEMENTS(varyings), GL_INTERLEAVED_ATTRIBS);

	ss.str("");
	Util::resolveFileLinking(ss, "./src/billboard-shader.glsl", "#include");
	billboardShader = new Shader(context, ss.str());

	context.setRasterizerDiscard(true);
	context.beginTransformFeedback(*transformShader, *feedback, GL_POINTS);

	context.drawArray(*transformShader, *inputBuffer, 1, GL_POINTS);

	context.endTransformFeedback();
	context.setRasterizerDiscard(false);
}

void ParticleSystem::drawParticle(const Particle& particle) {
	particleBuffer.push_back(particle);
}

void ParticleSystem::drawParticle(const glm::vec3& position,
		const glm::vec3& velocity, float timeToLive) {
	particleBuffer.emplace_back(position, velocity, timeToLive);
}

void ParticleSystem::update() {
	const uintptr numParticles = particleBuffer.size();

	context->setRasterizerDiscard(true);
	context->beginTransformFeedback(*transformShader, *feedback, GL_POINTS);

	if (numParticles > 0) {
		inputBuffer->update(&particleBuffer[0], numParticles * sizeof(Particle));
		context->drawArray(*transformShader, *inputBuffer,
				numParticles, GL_POINTS);
		particleBuffer.clear();
	}

	context->drawTransformFeedback(*transformShader, *feedback, GL_POINTS);

	context->endTransformFeedback();
	context->setRasterizerDiscard(false);

	feedback->swapBuffers();
	
	context->awaitFinish();
}

void ParticleSystem::draw(RenderTarget& target, Texture& texture, Sampler& sampler) {
	context->setBlending(RenderContext::BLEND_FUNC_SRC_ALPHA,
			RenderContext::BLEND_FUNC_DST_ALPHA);

	billboardShader->setSampler("billboard", texture, sampler, 0);
	context->drawTransformFeedback(target, *billboardShader, *feedback, GL_POINTS);

	context->setBlending(RenderContext::BLEND_FUNC_NONE, RenderContext::BLEND_FUNC_NONE);
}

ParticleSystem::~ParticleSystem() {
	delete feedback;
	delete inputBuffer;
	
	delete transformShader;
	delete billboardShader;
}
