#include "shader-storage-buffer.hpp"

#include <cstring>

ShaderStorageBuffer::ShaderStorageBuffer(RenderContext& context, uintptr dataSize,
			uint32 usage, uint32 blockBinding, const void* data)
		: context(&context)
		, bufferID(0)
		, blockBinding(blockBinding)
		, size(dataSize) {
	glGenBuffers(1, &bufferID);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, dataSize, data, usage);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, blockBinding, bufferID);
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

void* ShaderStorageBuffer::map(uint32 access) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);

	return glMapBuffer(GL_SHADER_STORAGE_BUFFER, access);
}

void* ShaderStorageBuffer::map(uintptr offset, uintptr size, uint32 access) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);

	return glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, access);
}

void ShaderStorageBuffer::unmap() {
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
	glDeleteBuffers(1, &bufferID);
}
