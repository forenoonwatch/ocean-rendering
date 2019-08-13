#pragma once

#include "common.hpp"

#include <string>

class Shader;
class VertexArray;

class RenderContext {
	public:
		RenderContext();

		void clear();

		void draw(Shader& shader, VertexArray& vertexArray,
				uint32 primitive, uint32 numInstances = 1);

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
