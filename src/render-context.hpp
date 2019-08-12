#pragma once

#include "common.hpp"

#include <string>

class RenderContext {
	public:
		RenderContext();

		uint32 getVersion();
		std::string getShaderVersion();
	private:
		uint32 version;
		std::string shaderVersion;
};
