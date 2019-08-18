#pragma once

#include "texture.hpp"

class RenderTarget {
	public:
		inline RenderTarget(RenderContext& context)
				: context(&context)
				, bufferID(0)
				, width(0) 
				, height(0) {}

		RenderTarget(RenderContext& context, Texture& texture,
				uint32 width, uint32 height, uint32 attachmentType,
				uint32 attachmentNumber = 0);
		RenderTarget(RenderContext& context, Texture& texture,
				uint32 attachmentType, uint32 attachmentNumber = 0);

		void clear(uint32 flags);

		inline uint32 getID() { return bufferID; }

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		~RenderTarget();
	private:
		NULL_COPY_AND_ASSIGN(RenderTarget);

		RenderContext* context;
		uint32 bufferID;

		uint32 width;
		uint32 height;
};
