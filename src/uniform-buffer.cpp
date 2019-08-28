#include "uniform-buffer.hpp"

#include <cstring>

UniformBuffer::UniformBuffer(RenderContext& context, uintptr dataSize,
			uint32 usage, uint32 blockBinding, const void* data)
		: context(&context)
		, bufferID(0)
		, blockBinding(blockBinding)
		, size(dataSize) {
	glGenBuffers(1, &bufferID);

	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
	glBufferData(GL_UNIFORM_BUFFER, dataSize, data, usage);

	glBindBufferBase(GL_UNIFORM_BUFFER, blockBinding, bufferID);
}

void UniformBuffer::update(const void* data, uintptr dataSize) {
	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);

	void* dest = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	std::memcpy(dest, data, dataSize);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffer::update(const void* data, uintptr offset, uintptr dataSize) {
	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);

	void* dest = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	std::memcpy((void*)((uintptr)dest + offset), data, dataSize);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void* UniformBuffer::map() {
	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);

	return glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
}

void* UniformBuffer::map(uintptr offset, uintptr size) {
	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);

	return glMapBufferRange(GL_UNIFORM_BUFFER, offset, size, GL_WRITE_ONLY);
}

void UniformBuffer::unmap() {
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &bufferID);
}
