#pragma once

#include <vector>
#include <unordered_map>

#include <GLM/glm.hpp>

#include "uniform-buffer.hpp"
#include "shader-storage-buffer.hpp"

#include "sampler.hpp"
#include "texture.hpp"
#include "cube-map.hpp"

class Shader {
	public:
		Shader(RenderContext& context, const std::string& text);

		void setUniformBuffer(const std::string& name, UniformBuffer& buffer);

		void setShaderStorageBuffer(const std::string& name,
				ShaderStorageBuffer& buffer);

		void setSampler(const std::string& name, Texture& texture,
				Sampler& sampler, uint32 textureUnit);
		void setSampler(const std::string& name, CubeMap& cubeMap,
				Sampler& sampler, uint32 textureUnit);

		void bindComputeTexture(Texture& texture, uint32 unit,
				uint32 access, uint32 internalFormat);

		void setInt(const std::string& name, int32 value);
		void setFloat(const std::string& name, float value);

		void setVector2f(const std::string& name, const glm::vec2& value);
		void setVector3f(const std::string& name, const glm::vec3& value);

		void setMatrix4f(const std::string& name, const glm::mat4& value);

		inline int32 getUniform(const std::string& name) { return uniformMap[name]; }

		inline uint32 getID() { return programID; }

		~Shader();
	private:
		NULL_COPY_AND_ASSIGN(Shader);

		RenderContext* context;
		
		uint32 programID;

		std::vector<uint32> shaders;
		std::unordered_map<std::string, int32> uniformBlockMap;
		std::unordered_map<std::string, int32> samplerMap;
		std::unordered_map<std::string, int32> uniformMap;
};

