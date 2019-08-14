#include "shader-storage-buffer.hpp"

#include <cstring>

#include <GL/glew.h>
#include <GL/gl.h>

ShaderStorageBuffer::ShaderStorageBuffer(RenderContext& context, uintptr dataSize,
			uint32 usage, const void* data)
		: context(&context)
		, bufferID(0)
		, size(dataSize) {
	glGenBuffers(1, &bufferID);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, dataSize, data, usage);
}

void ShaderStorageBuffer::update(const void* data, uintptr dataSize) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);

	void* dest = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	std::memcpy(dest, data, dataSize);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ShaderStorageBuffer::update(const void* data, uintptr offset, uintptr dataSize) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);

	void* dest = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	std::memcpy((void*)((uintptr)dest + offset), data, dataSize);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
	glDeleteBuffers(1, &bufferID);
}
