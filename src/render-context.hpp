#pragma once

#include "common.hpp"

#include <string>
#include <GL/glew.h>

class Shader;
class VertexArray;

class RenderContext {
	public:
		RenderContext();

		void clear();
		void awaitFinish();

		void draw(Shader& shader, VertexArray& vertexArray,
				uint32 primitive, uint32 numInstances = 1);

		void compute(Shader& shader, uint32 numGroupsX,
				uint32 numGroupsY = 1, uint32 numGroupsZ = 1);

		uint32 getVersion();
		std::string getShaderVersion();

		void setShader(uint32);
		void setVertexArray(uint32);
	private:
		uint32 version;
		std::string shaderVersion;

		uint32 currentShader;
		uint32 currentVertexArray;
};
