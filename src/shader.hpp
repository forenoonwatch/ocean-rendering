#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "render-context.hpp"

class Shader {
	public:
		Shader(RenderContext& context, const std::string& text);

		inline uint32 getID() { return programID; }

		~Shader();
	private:
		RenderContext* context;
		
		uint32 programID;

		std::vector<uint32> shaders;
		std::unordered_map<std::string, int32> uniformMap;
		std::unordered_map<std::string, int32> samplerMap;
};
