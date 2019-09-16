#include "wake-system.hpp"

#include "deferred-render-target.hpp"

#include "indexed-model.hpp"
#include "util.hpp"

static void initCube(IndexedModel&);

WakeSystem::WakeSystem(RenderContext& context, Texture& displacementMap,
			Sampler& displacementSampler, uintptr wakeBufferSize,
			uintptr inputBufferSize)
		: context(&context)
		, feedbackQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
		, displacementMap(&displacementMap)
		, displacementSampler(&displacementSampler) {
	const uint32 elementSizes[] = {1, 16, 16, 16};
	const char* varyings[] = {"ttl1", "transform01", "transform11", "transform21"};

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

void WakeSystem::drawWake(const glm::mat4& transform) {
	wakeBuffer.emplace_back(1.f, transform);
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
	wakeShader->setSampler("colorBuffer", target.getColorBuffer(), target.getSampler(), 1);
	wakeShader->setSampler("normalBuffer", target.getNormalBuffer(), target.getSampler(), 2);
	wakeShader->setSampler("lightingBuffer", target.getLightingBuffer(), target.getSampler(), 3);
	
	wakeShader->setSampler("diffuse", texture, sampler, 4);

	uint32 numDrawn = feedbackQuery.getResultInt();
	//DEBUG_LOG_TEMP("%d primitives drawn", numDrawn);

	//context->drawTransformFeedback(target.getTarget(), *wakeShader, *feedback, GL_POINTS);
	context->draw(target.getTarget(), *wakeShader, *(cubes[feedback->getReadIndex()]),
			GL_TRIANGLES, numDrawn);
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
