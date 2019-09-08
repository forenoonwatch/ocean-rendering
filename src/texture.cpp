#include "texture.hpp"

static uint32 calcInternalFormat(uint32 pixelFormat, bool compressed);

Texture::Texture(RenderContext& context, uint32 width,
			uint32 height, uint32 internalPixelFormat,
			const void* data, uint32 pixelFormat, uint32 dataType,
			bool compressed, bool mipMaps)
		: context(&context)
		, textureID(-1)
		, width(width)
		, height(height)
		, internalFormat(calcInternalFormat(internalPixelFormat, compressed))
		, compressed(compressed)
		, mipMaps(mipMaps) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
			width, height, 0, pixelFormat, dataType, data);

	if (mipMaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}
}

Texture::Texture(RenderContext& context, const Bitmap& bitmap,
			uint32 internalPixelFormat)
		: Texture(context, bitmap.getWidth(), bitmap.getHeight(),
			   internalPixelFormat, bitmap.getPixels())	{}

Texture::Texture(RenderContext& context, const DDSTexture& ddsTexture)
		: context(&context)
		, textureID(-1)
		, width(ddsTexture.getWidth())
		, height(ddsTexture.getHeight())
		, internalFormat(ddsTexture.getInternalPixelFormat())
		, compressed(ddsTexture.isCompressed())
		, mipMaps(ddsTexture.getMipMapCount() > 1) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	if (compressed) {
		const uint32 blockSize = (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;

		uint32 offset = 0;
		uint32 w = width, h = height;

		for (uint32 level = 0; level < ddsTexture.getMipMapCount()
				&& (w || h); ++level) {
			uint32 size = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;

			glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat,
					w, h, 0, size, ddsTexture.getData() + offset);

			offset += size;
			w /= 2;
			h /= 2;
		}
	}
	else {
		uint32 blockSize, dataType;

		if (internalFormat == GL_RGBA32F) {
			blockSize = 16;
			dataType = GL_FLOAT;
		}
		else {
			blockSize = 8;
			dataType = GL_HALF_FLOAT;
		}

		uint32 offset = 0;
		uint32 w = width, h = height;

		for (uint32 level = 0; level < ddsTexture.getMipMapCount()
				&& (w || h); ++level) {
			uint32 size = w * h * blockSize;

			glTexImage2D(GL_TEXTURE_2D, level, internalFormat,
					w, h, 0, GL_RGBA, dataType,
					ddsTexture.getData() + offset);

			offset += size;
			w /= 2;
			h /= 2;
		}
	}
}

Texture::~Texture() {
	glDeleteTextures(1, &textureID);
}

static uint32 calcInternalFormat(uint32 pixelFormat, bool compressed) {
	switch (pixelFormat) {
		case GL_RGB:
			return compressed ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
					: GL_RGB;
		case GL_RGBA:
			return compressed ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
					: GL_RGBA;
		case GL_RED:
		case GL_RG:
		case GL_RGBA32F:
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_STENCIL:
			return pixelFormat;
		default:
			DEBUG_LOG(LOG_ERROR, "Texture",
					"%d is not a valid pixel format", pixelFormat);
			return 0;
	}
}
