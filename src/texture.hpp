#pragma once

#include "render-context.hpp"

#include "bitmap.hpp"
#include "dds-texture.hpp"

class Texture {
	public:
		Texture(RenderContext& context, uint32 width,
				uint32 height, uint32 internalPixelFormat,
				const void* data = nullptr,
				uint32 pixelFormat = GL_RGBA,
				uint32 dataType = GL_UNSIGNED_BYTE,
				bool compressed = false, bool mipMaps = false);
		Texture(RenderContext& context, const Bitmap& bitmap,
				uint32 internalPixelFormat);
		Texture(RenderContext& context, const DDSTexture& ddsTexture);

		inline uint32 getID() { return textureID; }

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline uint32 getInternalFormat() const { return internalFormat; }

		inline bool isCompressed() const { return compressed; }
		inline bool hasMipMaps() const { return mipMaps; }

		~Texture();
	private:
		NULL_COPY_AND_ASSIGN(Texture);

		RenderContext* context;

		uint32 textureID;

		uint32 width;
		uint32 height;

		uint32 internalFormat;

		bool compressed;
		bool mipMaps;
};
