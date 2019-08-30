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

RenderTarget::RenderTarget(RenderContext& context, Texture& texture,
			uint32 attachmentType, uint32 attachmentNumber)
		: RenderTarget(context, texture, texture.getWidth(),
				texture.getHeight(), attachmentType, attachmentNumber) {}

void RenderTarget::clear(uint32 flags) {
	context->setRenderTarget(bufferID);

	glClear(flags);
}

void RenderTarget::drawTo(RenderTarget& target, uint32 mask,
		uint32 filter) {
	context->setRenderTarget(bufferID, GL_READ_FRAMEBUFFER);
	context->setRenderTarget(target.getID(), GL_DRAW_FRAMEBUFFER);

	glBlitFramebuffer(0, 0, width, height, 0, 0,
			target.getWidth(), target.getHeight(), mask, filter);
}

void RenderTarget::addTextureTarget(Texture& texture,
		uint32 attachmentType, uint32 attachmentNumber) {
	context->setRenderTarget(bufferID);

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType
			+ attachmentNumber, GL_TEXTURE_2D, texture.getID(), 0);
}

void RenderTarget::addRenderBuffer(RenderBuffer& buffer,
		uint32 attachmentType, uint32 attachmentNumber) {
	context->setRenderTarget(bufferID);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType
			+ attachmentNumber, GL_RENDERBUFFER, buffer.getID());
}

RenderTarget::~RenderTarget() {
	glDeleteFramebuffers(1, &bufferID);
}
