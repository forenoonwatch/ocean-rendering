#include "render-buffer.hpp"

RenderBuffer::RenderBuffer(RenderContext& context, uint32 width,
			uint32 height, uint32 internalFormat)
		: context(&context)
		, bufferID(0)
		, width(width)
		, height(height) {
	glGenRenderbuffers(1, &bufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, bufferID);

	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}

RenderBuffer::~RenderBuffer() {
	glDeleteRenderbuffers(1, &bufferID);
}
