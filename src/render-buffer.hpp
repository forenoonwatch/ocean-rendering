#pragma once

#include "render-context.hpp"

class RenderBuffer {
	public:
		RenderBuffer(RenderContext& context, uint32 width,
				uint32 height, uint32 internalFormat);

		inline uint32 getID() { return bufferID; }

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		~RenderBuffer();
	private:
		NULL_COPY_AND_ASSIGN(RenderBuffer);

		RenderContext* context;
		uint32 bufferID;

		uint32 width;
		uint32 height;
};
