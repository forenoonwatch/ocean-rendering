#include "render-context.hpp"

#include "shader.hpp"
#include "vertex-array.hpp"
#include "render-target.hpp"

#include "indexed-model.hpp"

inline static void initScreenQuad(IndexedModel&);

uint32 RenderContext::attachments[] = {GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

RenderContext::RenderContext()
		: version(0)
		, shaderVersion("")
		, currentShader(-1)
		, currentVertexArray(-1)
		, currentRenderSource(0)
		, currentRenderTarget(0) {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);

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
			glDrawElements(primitive, (GLsizei)vertexArray.getNumIndices(), GL_UNSIGNED_INT, 0);
			return;
		default:
			glDrawElementsInstanced(primitive, (GLsizei)vertexArray.getNumIndices(),
					GL_UNSIGNED_INT, 0, numInstances);
	}
}

void RenderContext::compute(Shader& shader, uint32 numGroupsX,
		uint32 numGroupsY, uint32 numGroupsZ) {
	setShader(shader.getID());
	glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void RenderContext::drawQuad(RenderTarget& target, Shader& shader) {
	setRenderTarget(target.getID());
	setShader(shader.getID());
	setVertexArray(screenQuad->getID());

	glDrawElements(GL_TRIANGLES, (GLsizei)screenQuad->getNumIndices(),
			GL_UNSIGNED_INT, 0);
}

void RenderContext::setDrawBuffers(uint32 numBuffers) {
	glDrawBuffers(numBuffers, attachments);
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
	screenQuadModel.allocateElement(2); // texCoord

	screenQuadModel.addElement2f(0, -1.f, -1.f);
	screenQuadModel.addElement2f(0, -1.f,  1.f);
	screenQuadModel.addElement2f(0,  1.f, -1.f);
	screenQuadModel.addElement2f(0,  1.f,  1.f);

	screenQuadModel.addElement2f(1, 0.f, 0.f);
	screenQuadModel.addElement2f(1, 0.f, 1.f);
	screenQuadModel.addElement2f(1, 1.f, 0.f);
	screenQuadModel.addElement2f(1, 1.f, 1.f);

	screenQuadModel.addIndices3i(2, 1, 0);
	screenQuadModel.addIndices3i(1, 2, 3);
}
