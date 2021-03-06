#include "wake-system.hpp"

#include "deferred-render-target.hpp"

#include <engine/rendering/indexed-model.hpp>
#include <engine/core/util.hpp>

static void initCube(IndexedModel&);

WakeSystem::WakeSystem(RenderContext& context, Texture& displacementMap,
			Sampler& displacementSampler, uintptr wakeBufferSize,
			uintptr inputBufferSize)
		: context(&context)
		, feedbackQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
		, displacementMap(&displacementMap)
		, displacementSampler(&displacementSampler) {
	const uint32 elementSizes[] = {4, 4, 16, 16, 16};
	const char* varyings[] = {"timeDriftData1", "transScale1",
			"transform01", "transform11", "transform21"};

	feedback = new TransformFeedback(context, ARRAY_SIZE_IN_ELEMENTS(elementSizes),
			elementSizes, wakeBufferSize);
	inputBuffer = new InputStreamBuffer(context, ARRAY_SIZE_IN_ELEMENTS(elementSizes),
			elementSizes, inputBufferSize);

	std::stringstream ss;
	Util::resolveFileLinking(ss, "./src/wake-update-shader.glsl", "#include");
	transformShader = new Shader(context, ss.str(), varyings,
			ARRAY_SIZE_IN_ELEMENTS(varyings), GL_INTERLEAVED_ATTRIBS);

	ss.str("");
	Util::resolveFileLinking(ss, "./src/wake-shader.glsl", "#include");
	wakeShader = new Shader(context, ss.str());

	IndexedModel cubeModel;
	initCube(cubeModel);

	cubes[0] = new VertexArray(context, cubeModel, *feedback, 0, GL_STATIC_DRAW);
	cubes[1] = new VertexArray(context, cubeModel, *(cubes[0]), *feedback, 1);

	context.setRasterizerDiscard(true);
	context.beginTransformFeedback(*transformShader, *feedback, GL_POINTS);

	context.drawArray(*transformShader, *inputBuffer, 1, GL_POINTS);

	context.endTransformFeedback();
	context.setRasterizerDiscard(false);
}

void WakeSystem::drawWake(const glm::vec2& driftVelocity,
		const glm::vec4& transScale, const glm::mat4& transform, float timeToLive) {
	wakeBuffer.emplace_back(timeToLive, driftVelocity, transScale, transform);
}

void WakeSystem::update() {
	const uintptr numWakes = wakeBuffer.size();

	transformShader->setSampler("displacement", *displacementMap, *displacementSampler, 0);

	context->setRasterizerDiscard(true);
	context->beginQuery(feedbackQuery);
	context->beginTransformFeedback(*transformShader, *feedback, GL_POINTS);

	if (numWakes > 0) {
		inputBuffer->update(&wakeBuffer[0], numWakes * sizeof(WakeInstance));
		context->drawArray(*transformShader, *inputBuffer,
				numWakes, GL_POINTS);
		wakeBuffer.clear();
	}

	context->drawTransformFeedback(*transformShader, *feedback, GL_POINTS);

	context->endTransformFeedback();
	context->endQuery(feedbackQuery);
	context->setRasterizerDiscard(false);

	feedback->swapBuffers();
	
	context->awaitFinish();
}

void WakeSystem::draw(DeferredRenderTarget& target, Texture& texture, Sampler& sampler) {
	wakeShader->setSampler("depthBuffer", target.getDepthBuffer(), target.getSampler(), 0);
	//wakeShader->setSampler("colorBuffer", target.getColorBuffer(), target.getSampler(), 1);
	wakeShader->setSampler("normalBuffer", target.getNormalBuffer(), target.getSampler(), 1);
	wakeShader->setSampler("lightingBuffer", target.getLightingBuffer(), target.getSampler(), 2);
	
	wakeShader->setSampler("diffuse", texture, sampler, 3);

	uint32 numDrawn = feedbackQuery.getResultInt();

	context->setBlending(RenderContext::BLEND_FUNC_SRC_ALPHA,
			RenderContext::BLEND_FUNC_ONE_MINUS_SRC_ALPHA);

	context->setWriteDepth(false);
	context->draw(target.getTarget(), *wakeShader, *(cubes[feedback->getReadIndex()]),
			GL_TRIANGLES, numDrawn);
	context->setWriteDepth(true);

	context->setBlending(RenderContext::BLEND_FUNC_NONE, RenderContext::BLEND_FUNC_NONE);
}

WakeSystem::~WakeSystem() {
	delete feedback;
	delete inputBuffer;
	
	delete transformShader;
	delete wakeShader;

	for (uint32 i = 0; i < 2; ++i) {
		delete cubes[i];
	}
}

static void initCube(IndexedModel& model) {
	model.allocateElement(3); // position
	model.allocateElement(16); // transform
	
	model.setInstancedElementStartIndex(1);

	for (float z = -1.f; z <= 1.f; z += 2.f) {
		for (float y = -1.f; y <= 1.f; y += 2.f) {
			for (float x = -1.f; x <= 1.f; x += 2.f) {
				model.addElement3f(0, x, y, z);
			}
		}
	}

	// back
	model.addIndices3i(2, 1, 0);
	model.addIndices3i(1, 2, 3);

	// front
	model.addIndices3i(4, 5, 6);
	model.addIndices3i(7, 6, 5);

	// bottom
	model.addIndices3i(0, 1, 4);
	model.addIndices3i(5, 4, 1);

	// top
	model.addIndices3i(6, 3, 2);
	model.addIndices3i(3, 6, 7);

	// left
	model.addIndices3i(6, 2, 0);
	model.addIndices3i(0, 4, 6);

	// right
	model.addIndices3i(7, 5, 1);
	model.addIndices3i(1, 3, 7);
}
