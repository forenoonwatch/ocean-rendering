#include "render-context.hpp"

#include <GL/glew.h>

RenderContext::RenderContext()
	: version(0) {}

uint32 RenderContext::getVersion() {
	if (version != 0) {
		return version;
	}

	int32 major, minor;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	version = (uint32)(major * 100 + minor * 10);

	return version;
}

std::string RenderContext::getShaderVersion() {
	if (!shaderVersion.empty()) {
		return shaderVersion;
	}

	uint32 ver = getVersion();

	if (ver >= 330) {
		shaderVersion = std::to_string(ver);
	}
	else if (ver >= 320) {
		shaderVersion = "330"; // really should be 150 but bugs on my laptop
	}
	else if (ver >= 310) {
		shaderVersion = "140";
	}
	else if (ver >= 300) {
		shaderVersion = "130";
	}
	else if (ver >= 210) {
		shaderVersion = "120";
	}
	else {
		shaderVersion = "110";
	}

	return shaderVersion;
}
