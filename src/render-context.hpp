#pragma once

#include "common.hpp"

#include <string>
#include <GL/glew.h>

class Shader;
class VertexArray;
class RenderTarget;

class RenderContext {
	public:
		RenderContext();

		void awaitFinish();

		void draw(RenderTarget& target, Shader& shader, VertexArray& vertexArray,
				uint32 primitive, uint32 numInstances = 1);
		void compute(Shader& shader, uint32 numGroupsX,
				uint32 numGroupsY = 1, uint32 numGroupsZ = 1);

		void drawQuad(RenderTarget& target, Shader& shader);

		void setDrawBuffers(uint32 numBuffers);

		void setWriteDepth(bool writeDepth);

		uint32 getVersion();
		std::string getShaderVersion();

		void setShader(uint32);
		void setVertexArray(uint32);

		void setRenderTarget(uint32 fbo, uint32 bufferType = GL_FRAMEBUFFER);

		~RenderContext();
	private:
		NULL_COPY_AND_ASSIGN(RenderContext);

		VertexArray* screenQuad;

		uint32 version;
		std::string shaderVersion;

		uint32 currentShader;
		uint32 currentVertexArray;

		uint32 currentRenderSource;
		uint32 currentRenderTarget;

		static uint32 attachments[4];
};
