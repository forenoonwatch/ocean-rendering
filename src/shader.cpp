#include "shader.hpp"

#include <GLM/gtc/type_ptr.hpp>

#define SHADER_INFO_LOG_SIZE	1024

static bool addShader(GLuint program, const std::string& text,
		GLenum type, std::vector<uint32>& shaders);
static bool checkShaderError(uint32 shader, GLenum flag,
		bool isProgram, const std::string& errorMessage);

static void addShaderUniforms(GLuint program,
		std::unordered_map<std::string, int32>& uniformBlockMap,
		std::unordered_map<std::string, int32>& samplerMap,
		std::unordered_map<std::string, int32>& uniformMap); 

Shader::Shader(RenderContext& context, const std::string& text)
		: context(&context)
		, programID(glCreateProgram()) {
	std::string version = "#version " + context.getShaderVersion()
		+ "\n#define GLSL_VERSION " + context.getShaderVersion();

	if (text.find("CS_BUILD") != std::string::npos) {
		std::string computeShaderText = version
			+ "\n#define CS_BUILD\n" + text;

		if (!addShader(programID, computeShaderText, GL_COMPUTE_SHADER, shaders)) {
			throw std::runtime_error("Failed to load compute shader");
		}
	}
	else {
		std::string vertexShaderText = version
			+ "\n#define VS_BUILD\n" + text;
		std::string fragmentShaderText = version
			+ "\n#define FS_BUILD\n" + text;

		if (!addShader(programID, vertexShaderText, GL_VERTEX_SHADER, shaders)) {
			throw std::runtime_error("Failed to load vertex shader");
		}

		if (!addShader(programID, fragmentShaderText, GL_FRAGMENT_SHADER, shaders)) {
			throw std::runtime_error("Failed to load fragment shader");
		}

		if (text.find("GS_BUILD") != std::string::npos) {
			std::string geomShaderText = version
				+ "\n#define GS_BUILD\n" + text;

			if (!addShader(programID, geomShaderText, GL_GEOMETRY_SHADER, shaders)) {
				throw std::runtime_error("Failed to load geometry shader");
			}
		}
	}

	glLinkProgram(programID);

	if (checkShaderError(programID, GL_LINK_STATUS, true,
				"Error linking shader program")) {
		throw std::runtime_error("Error linking shader program");
	}

	glValidateProgram(programID);

	if (checkShaderError(programID, GL_VALIDATE_STATUS, true,
				"Invalid shader program")) {
		throw std::runtime_error("Invalid shader program");
	}

	// TODO: add attributes
	addShaderUniforms(programID, uniformBlockMap,
			samplerMap, uniformMap);
}

void Shader::setUniformBuffer(const std::string& name, UniformBuffer& buffer) {
	glUniformBlockBinding(programID, uniformBlockMap[name],
			buffer.getBlockBinding());
}

void Shader::setShaderStorageBuffer(const std::string& name,
		ShaderStorageBuffer& buffer) {
	glUniformBlockBinding(programID, uniformBlockMap[name],
			buffer.getBlockBinding());
}

void Shader::setSampler(const std::string& name, Texture& texture,
		Sampler& sampler, uint32 textureUnit) {
	context->setShader(programID);

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture.getID());
	glBindSampler(textureUnit, sampler.getID());
	glUniform1i(samplerMap[name], textureUnit);
}

void Shader::setSampler(const std::string& name, CubeMap& cubeMap,
		Sampler& sampler, uint32 textureUnit) {
	context->setShader(programID);

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.getID());
	glBindSampler(textureUnit, sampler.getID());
	glUniform1i(samplerMap[name], textureUnit);
}

void Shader::bindComputeTexture(Texture& texture, uint32 unit,
		uint32 access, uint32 internalFormat) {
	context->setShader(programID);
	glBindImageTexture(unit, texture.getID(), 0, false, 0,
			access, internalFormat);
}

void Shader::setInt(const std::string& name, int32 value) {
	context->setShader(programID);
	glUniform1i(uniformMap[name], value);
}

void Shader::setFloat(const std::string& name, float value) {
	context->setShader(programID);
	glUniform1f(uniformMap[name], value);
}

void Shader::setVector2f(const std::string& name, const glm::vec2& value) {
	context->setShader(programID);
	glUniform2fv(uniformMap[name], 1, glm::value_ptr(value));
}

void Shader::setVector3f(const std::string& name, const glm::vec3& value) {
	context->setShader(programID);
	glUniform3fv(uniformMap[name], 1, glm::value_ptr(value));
}

Shader::~Shader() {
	for (auto it = std::begin(shaders), end = std::end(shaders);
			it != end; ++it) {
		glDetachShader(programID, *it);
		glDeleteShader(*it);
	}

	glDeleteProgram(programID);
}

static bool addShader(GLuint program, const std::string& text,
		GLenum type, std::vector<uint32>& shaders) {
	uint32 shader = glCreateShader(type);

	if (shader == 0) {
		DEBUG_LOG("Shader", LOG_ERROR, "Error creating shader type %d", type);
		return false;
	}

	const GLchar* p[1] = { text.c_str() };
	GLint lengths[1] = { (GLint)text.length() };

	glShaderSource(shader, 1, p, lengths);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (!status) {
		GLchar infoLog[SHADER_INFO_LOG_SIZE];
		glGetShaderInfoLog(shader, SHADER_INFO_LOG_SIZE, nullptr, infoLog);

		DEBUG_LOG("Shader", LOG_ERROR, "Error compiling shader type %d: '%s'\n",
				type, infoLog);

		return false;
	}

	glAttachShader(program, shader);
	shaders.push_back(shader);

	return true;
}

static bool checkShaderError(uint32 shader, GLenum flag,
		bool isProgram, const std::string& errorMessage) {
	GLint status = 0;
	GLchar error[SHADER_INFO_LOG_SIZE];

	if (isProgram) {
		glGetProgramiv(shader, flag, &status);
	}
	else {
		glGetShaderiv(shader, flag, &status);
	}

	if (!status) {
		if (isProgram) {
			glGetProgramInfoLog(shader, SHADER_INFO_LOG_SIZE, nullptr, error);
		}
		else {
			glGetShaderInfoLog(shader, SHADER_INFO_LOG_SIZE, nullptr, error);
		}

		DEBUG_LOG("Shader", LOG_ERROR, "%s: '%s'\n",
				errorMessage.c_str(), error);

		return true;
	}

	return false;
}

static void addShaderUniforms(GLuint program,
		std::unordered_map<std::string, int32>& uniformBlockMap,
		std::unordered_map<std::string, int32>& samplerMap,
		std::unordered_map<std::string, int32>& uniformMap) {
	GLint numBlocks;
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

	for (int32 block = 0; block < numBlocks; ++block) {
		GLint nameLen;
		glGetActiveUniformBlockiv(program, block,
				GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

		std::vector<GLchar> name(nameLen);
		glGetActiveUniformBlockName(program, block,
				nameLen, nullptr, &name[0]);

		std::string uniformBlockName((char*)&name[0], nameLen - 1);
		uniformBlockMap[uniformBlockName] = glGetUniformBlockIndex(program, &name[0]);
	}

	GLint numUniforms = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

	std::vector<GLchar> uniformName(256);

	for (int32 uniform = 0; uniform < numUniforms; ++uniform) {
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;

		glGetActiveUniform(program, uniform, uniformName.size(),
				&actualLength, &arraySize, &type, &uniformName[0]);

		std::string name((char*)&uniformName[0], actualLength);

		if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE) {
			samplerMap[name] = glGetUniformLocation(program, (char*)&uniformName[0]);
		}
		else {
			uniformMap[name] = glGetUniformLocation(program, (char*)&uniformName[0]);
		}
	}
}
