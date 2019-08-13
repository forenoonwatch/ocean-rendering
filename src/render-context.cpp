#include "render-context.hpp"

#include "shader.hpp"
#include "vertex-array.hpp"

#include <GL/glew.h>

RenderContext::RenderContext()
		: version(0)
		, shaderVersion("")
		, currentShader(-1)
		, currentVertexArray(-1) {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
}

void RenderContext::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderContext::draw(Shader& shader, VertexArray& vertexArray,
		uint32 primitive, uint32 numInstances) {
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
