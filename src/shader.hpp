#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "render-context.hpp"

class Shader {
	public:
		Shader(RenderContext& context, const std::string& text);

		inline uint32_t getID() { return programID; }

		~Shader();
	private:
		RenderContext* context;
		
		uint32_t programID;

		std::vector<uint32_t> shaders;
		std::unordered_map<std::string, int32_t> uniformMap;
		std::unordered_map<std::string, int32_t> samplerMap;
};
