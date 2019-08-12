#include "shader.hpp"

#include <GL/glew.h>
#include <GL/gl.h>

#define SHADER_INFO_LOG_SIZE	1024

static bool addShader(GLuint program, const std::string& text,
		GLenum type, std::vector<uint32>& shaders);
static bool checkShaderError(uint32 shader, GLenum flag,
		bool isProgram, const std::string& errorMessage);

Shader::Shader(RenderContext& context, const std::string& text)
		: context(&context)
		, programID(glCreateProgram()) {
	std::string version = "#version " + context.getShaderVersion()
		+ "\n#define GLSL_VERSION " + context.getShaderVersion();
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
	// TODO: add uniforms
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
		// TODO: log error
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
		// TODO: write info log
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

		// TODO: write info log
		return true;
	}

	return false;
}
