#include "render-target.hpp"

RenderTarget::RenderTarget(RenderContext& context, Texture& texture,
			uint32 width, uint32 height, uint32 attachmentType,
			uint32 attachmentNumber)
		: context(&context)
		, bufferID(-1)
		, width(width)
		, height(height) {
	glGenFramebuffers(1, &bufferID);
	context.setRenderTarget(bufferID);

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType
			+ attachmentNumber, GL_TEXTURE_2D, texture.getID(), 0);
}

void RenderTarget::clear(uint32 flags) {
	context->setRenderTarget(bufferID);
	context->clear(flags);
}

RenderTarget::RenderTarget(RenderContext& context, Texture& texture,
			uint32 attachmentType, uint32 attachmentNumber)
		: RenderTarget(context, texture, width,
				height, attachmentType, attachmentNumber) {}

RenderTarget::~RenderTarget() {
	glDeleteFramebuffers(1, &bufferID);
}
