#pragma once

#include "common.hpp"

#include <string>
#include <GL/glew.h>

class Shader;

class VertexArray;
class TransformFeedback;
class InputStreamBuffer;

class RenderTarget;

class RenderQuery;

class RenderContext {
	public:
		enum BlendFunc {
			BLEND_FUNC_NONE,
			BLEND_FUNC_ZERO = GL_ZERO,
			BLEND_FUNC_ONE = GL_ONE,
			BLEND_FUNC_SRC_ALPHA = GL_SRC_ALPHA,
			BLEND_FUNC_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
			BLEND_FUNC_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
			BLEND_FUNC_DST_ALPHA = GL_DST_ALPHA
		};

		RenderContext();

		void awaitFinish();

		void draw(RenderTarget& target, Shader& shader, VertexArray& vertexArray,
				uint32 primitive, uint32 numInstances = 1);

		void drawArray(RenderTarget& target, Shader& shader, VertexArray& vertexArray,
				uint32 bufferIndex, uint32 primitive, uint32 numInstances = 1,
				uint32 numElements = 0);
		void drawArray(Shader& shader, VertexArray& vertexArray,
				uint32 bufferIndex, uint32 primitive, uint32 numInstances = 1,
				uint32 numElements = 0);
		void drawArray(Shader& shader, InputStreamBuffer& isb, uint32 numElements,
				uint32 primitive);

		void drawTransformFeedback(RenderTarget& target, Shader& shader,
				TransformFeedback& transformFeedback, uint32 primitive);
		void drawTransformFeedback(Shader& shader, TransformFeedback& transformFeedback,
				uint32 primitive);
		
		void drawQuad(RenderTarget& target, Shader& shader);

		void compute(Shader& shader, uint32 numGroupsX,
				uint32 numGroupsY = 1, uint32 numGroupsZ = 1);

		void beginTransformFeedback(Shader& shader, TransformFeedback& tfb,
				uint32 primitive);
		void endTransformFeedback();

		void beginQuery(RenderQuery& query);
		void endQuery(RenderQuery& query);

		void setDrawBuffers(uint32 numBuffers);

		void setWriteDepth(bool writeDepth);
		void setRasterizerDiscard(bool discard);
		void setBlending(enum BlendFunc srcBlend, enum BlendFunc destBlend);

		uint32 getVersion();
		std::string getShaderVersion();

		void setShader(uint32);
		void setVertexArray(uint32);
		void setTransformFeedback(uint32);

		void setRenderTarget(uint32 fbo, uint32 bufferType = GL_FRAMEBUFFER);

		~RenderContext();
	private:
		NULL_COPY_AND_ASSIGN(RenderContext);

		VertexArray* screenQuad;

		uint32 version;
		std::string shaderVersion;

		enum BlendFunc currentSourceBlend;
		enum BlendFunc currentDestBlend;

		uint32 currentShader;
		uint32 currentVertexArray;
		uint32 currentTFB;

		uint32 currentRenderSource;
		uint32 currentRenderTarget;

		static uint32 attachments[4];
};
