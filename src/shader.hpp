#pragma once

#include <vector>
#include <unordered_map>

#include "uniform-buffer.hpp"
#include "sampler.hpp"
#include "texture.hpp"

class Shader {
	public:
		Shader(RenderContext& context, const std::string& text);

		void setUniformBuffer(const std::string& name,
				UniformBuffer& buffer, uint32 index, uint32 block);
		inline void setUniformBuffer(const std::string& name,
				UniformBuffer& buffer, uint32 index);

		void setSampler(const std::string& name, Texture& texture,
				Sampler& sampler, uint32 textureUnit);

		void bindComputeTexture(Texture& texture, uint32 unit,
				uint32 access, uint32 internalFormat);

		inline int32 getUniform(const std::string& name) { return uniformMap[name]; }

		inline uint32 getID() { return programID; }

		~Shader();
	private:
		RenderContext* context;
		
		uint32 programID;

		std::vector<uint32> shaders;
		std::unordered_map<std::string, int32> uniformBlockMap;
		std::unordered_map<std::string, int32> samplerMap;
		std::unordered_map<std::string, int32> uniformMap;
};

inline void Shader::setUniformBuffer(const std::string& name,
		UniformBuffer& buffer, uint32 index) {
	setUniformBuffer(name, buffer, index, index);
}
