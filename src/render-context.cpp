#include "render-context.hpp"

#include "shader.hpp"
#include "vertex-array.hpp"
#include "transform-feedback.hpp"
#include "input-stream-buffer.hpp"

#include "render-target.hpp"

#include "indexed-model.hpp"

inline static void initScreenQuad(IndexedModel&);

static void GLAPIENTRY errorCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

uint32 RenderContext::attachments[] = {GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

RenderContext::RenderContext()
		: version(0)
		, shaderVersion("")
		, currentSourceBlend(BLEND_FUNC_NONE)
		, currentDestBlend(BLEND_FUNC_NONE)
		, currentShader(0)
		, currentVertexArray(0)
		, currentTFB(0)
		, currentRenderSource(0)
		, currentRenderTarget(0) {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(errorCallback, 0);

	IndexedModel screenQuadModel;
	initScreenQuad(screenQuadModel);

	screenQuad = new VertexArray(*this, screenQuadModel, GL_STATIC_DRAW);
}

void RenderContext::awaitFinish() {
	glFinish();
}

void RenderContext::draw(RenderTarget& target, Shader& shader,
		VertexArray& vertexArray, uint32 primitive, uint32 numInstances) {
	setRenderTarget(target.getID());
	setShader(shader.getID());
	setVertexArray(vertexArray.getID());

	switch (numInstances) {
		case 0:
			return;
		case 1:
			glDrawElements(primitive, (GLsizei)vertexArray.getNumElements(), GL_UNSIGNED_INT, 0);
			return;
		default:
			glDrawElementsInstanced(primitive, (GLsizei)vertexArray.getNumElements(),
					GL_UNSIGNED_INT, 0, numInstances);
	}
}

void RenderContext::drawArray(RenderTarget& target, Shader& shader,
		VertexArray& vertexArray, uint32 bufferIndex, uint32 primitive,
		uint32 numInstances, uint32 numElements) {
	setRenderTarget(target.getID());
	setShader(shader.getID());
	setVertexArray(vertexArray.getID());

	glBindBuffer(GL_ARRAY_BUFFER, vertexArray.getBuffer(bufferIndex));

	if (numElements == 0) {
		numElements = vertexArray.getNumElements();
	}

	switch (numInstances) {
		case 0:
			return;
		case 1:
			glDrawArrays(primitive, 0, numElements);
			return;
		default:
			glDrawArraysInstanced(primitive, 0, numElements, numInstances);
	}
}

void RenderContext::drawArray(Shader& shader, VertexArray& vertexArray,
		uint32 bufferIndex, uint32 primitive, uint32 numInstances,
		uint32 numElements) {
	setShader(shader.getID());
	setVertexArray(vertexArray.getID());

	glBindBuffer(GL_ARRAY_BUFFER, vertexArray.getBuffer(bufferIndex));

	if (numElements == 0) {
		numElements = vertexArray.getNumElements();
	}

	switch (numInstances) {
		case 0:
			return;
		case 1:
			glDrawArrays(primitive, 0, numElements);
			return;
		default:
			glDrawArraysInstanced(primitive, 0, numElements, numInstances);
	}
}

void RenderContext::drawArray(Shader& shader, InputStreamBuffer& isb,
		uint32 numElements, uint32 primitive) {
	setShader(shader.getID());
	setVertexArray(isb.getReadArray());

	glBindBuffer(GL_ARRAY_BUFFER, isb.getReadBuffer());

	glDrawArrays(primitive, 0, numElements);
}

void RenderContext::drawTransformFeedback(RenderTarget& target, Shader& shader,
		TransformFeedback& transformFeedback, uint32 primitive) {
	setRenderTarget(target.getID());
	setShader(shader.getID());
	setVertexArray(transformFeedback.getReadArray());

	glDrawTransformFeedback(primitive, transformFeedback.getReadFeedback());
}

void RenderContext::drawTransformFeedback(Shader& shader,
		TransformFeedback& transformFeedback, uint32 primitive) {
	setShader(shader.getID());
	setVertexArray(transformFeedback.getReadArray());

	glDrawTransformFeedback(primitive, transformFeedback.getReadFeedback());
}

void RenderContext::drawQuad(RenderTarget& target, Shader& shader) {
	setRenderTarget(target.getID());
	setShader(shader.getID());
	setVertexArray(screenQuad->getID());

	glDrawElements(GL_TRIANGLES, (GLsizei)screenQuad->getNumElements(),
			GL_UNSIGNED_INT, 0);
}

void RenderContext::compute(Shader& shader, uint32 numGroupsX,
		uint32 numGroupsY, uint32 numGroupsZ) {
	setShader(shader.getID());
	glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void RenderContext::beginTransformFeedback(Shader& shader, TransformFeedback& tfb,
		uint32 primitive) {
	setShader(shader.getID());
	setTransformFeedback(tfb.getWriteFeedback());

	glBeginTransformFeedback(primitive);
}

void RenderContext::endTransformFeedback() {
	glEndTransformFeedback();
}

void RenderContext::setDrawBuffers(uint32 numBuffers) {
	glDrawBuffers(numBuffers, attachments);
}

void RenderContext::setWriteDepth(bool writeDepth) {
	glDepthMask(writeDepth);
}

void RenderContext::setRasterizerDiscard(bool discard) {
	if (discard) {
		glEnable(GL_RASTERIZER_DISCARD);
	}
	else {
		glDisable(GL_RASTERIZER_DISCARD);
	}
}

void RenderContext::setBlending(enum BlendFunc srcBlend,
		enum BlendFunc destBlend) {
	if (srcBlend == currentSourceBlend && destBlend == currentDestBlend) {
		return;
	}

	if (srcBlend == BLEND_FUNC_NONE || destBlend == BLEND_FUNC_NONE) {
		glDisable(GL_BLEND);
	}
	else if (currentSourceBlend == BLEND_FUNC_NONE 
			|| currentDestBlend == BLEND_FUNC_NONE) {
		glEnable(GL_BLEND);
		glBlendFunc(srcBlend, destBlend);
	}
	else {
		glBlendFunc(srcBlend, destBlend);
	}

	currentSourceBlend = srcBlend;
	currentDestBlend = destBlend;
}

uint32 RenderContext::getVersion() {
	if (version != 0) {
		return version;
	}

	int32 major, minor;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	version = (uint32)(major * 100 + minor * 10);

	return version;
}

std::string RenderContext::getShaderVersion() {
	if (!shaderVersion.empty()) {
		return shaderVersion;
	}

	uint32 ver = getVersion();

	if (ver >= 330) {
		shaderVersion = std::to_string(ver);
	}
	else if (ver >= 320) {
		shaderVersion = "330"; // really should be 150 but bugs on my laptop
	}
	else if (ver >= 310) {
		shaderVersion = "140";
	}
	else if (ver >= 300) {
		shaderVersion = "130";
	}
	else if (ver >= 210) {
		shaderVersion = "120";
	}
	else {
		shaderVersion = "110";
	}

	return shaderVersion;
}

void RenderContext::setShader(uint32 shader) {
	if (currentShader != shader) {
		currentShader = shader;
		glUseProgram(shader);
	}
}

void RenderContext::setVertexArray(uint32 vao) {
	if (currentVertexArray != vao) {
		currentVertexArray = vao;
		glBindVertexArray(vao);
	}
}

void RenderContext::setTransformFeedback(uint32 tfb) {
	if (currentTFB != tfb) {
		currentTFB = tfb;
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
	}
}

void RenderContext::setRenderTarget(uint32 fbo, uint32 bufferType) {
	switch (bufferType) {
		case GL_FRAMEBUFFER:
			if (currentRenderSource != fbo || currentRenderTarget != fbo) {
				currentRenderSource = fbo;
				currentRenderTarget = fbo;

				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			}

			return;
		case GL_READ_FRAMEBUFFER:
			if (currentRenderSource != fbo) {
				currentRenderSource = fbo;

				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
			}

			return;
		case GL_DRAW_FRAMEBUFFER:
			if (currentRenderTarget != fbo) {
				currentRenderTarget = fbo;

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			}

			return;
	}
}

RenderContext::~RenderContext() {
	delete screenQuad;
}

inline static void initScreenQuad(IndexedModel& screenQuadModel) {
	screenQuadModel.allocateElement(2); // position

	screenQuadModel.addElement2f(0, -1.f, -1.f);
	screenQuadModel.addElement2f(0, -1.f,  1.f);
	screenQuadModel.addElement2f(0,  1.f, -1.f);
	screenQuadModel.addElement2f(0,  1.f,  1.f);

	screenQuadModel.addIndices3i(2, 1, 0);
	screenQuadModel.addIndices3i(1, 2, 3);
}

static void GLAPIENTRY errorCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		return;
	}

	DEBUG_LOG_TEMP2(message);
}
