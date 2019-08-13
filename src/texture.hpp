#pragma once

#include "render-context.hpp"
#include "bitmap.hpp"

class Texture {
	public:
		Texture(RenderContext& context, uint32 width,
				uint32 height, uint32 internalPixelFormat,
				bool storage = false, const void* data = nullptr);
		Texture(RenderContext& context, const Bitmap& bitmap,
				uint32 internalPixelFormat);

		inline uint32 getID() { return textureID; }

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline uint32 getInternalFormat() const { return internalFormat; }

		~Texture();
	private:
		RenderContext* context;

		uint32 textureID;

		uint32 width;
		uint32 height;

		uint32 internalFormat;
};
