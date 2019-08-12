#include "uniform-buffer.hpp"

#include <cstring>

#include <GL/glew.h>
#include <GL/gl.h>

UniformBuffer::UniformBuffer(RenderContext& context, uintptr dataSize,
			uint32 usage, const void* data)
		: context(&context)
		, bufferID(0)
		, size(dataSize) {
	glGenBuffers(1, &bufferID);

	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
	glBufferData(GL_UNIFORM_BUFFER, dataSize, data, usage);
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

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &bufferID);
}
