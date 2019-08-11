#pragma once

#include <string>

class RenderContext {
	public:
		RenderContext();

		uint32_t getVersion();
		std::string getShaderVersion();
	private:
		uint32_t version;
		std::string shaderVersion;
};
