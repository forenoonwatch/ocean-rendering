#pragma once

#include <vector>
#include <unordered_map>

#include "uniform-buffer.hpp"

class Shader {
	public:
		Shader(RenderContext& context, const std::string& text);

		void setUniformBuffer(const std::string& name,
				UniformBuffer& buffer, uint32 index, uint32 block);
		inline void setUniformBuffer(const std::string& name,
				UniformBuffer& buffer, uint32 index);

		inline uint32 getID() { return programID; }

		~Shader();
	private:
		RenderContext* context;
		
		uint32 programID;

		std::vector<uint32> shaders;
		std::unordered_map<std::string, int32> uniformMap;
		std::unordered_map<std::string, int32> samplerMap;
};

inline void Shader::setUniformBuffer(const std::string& name,
		UniformBuffer& buffer, uint32 index) {
	setUniformBuffer(name, buffer, index, index);
}
