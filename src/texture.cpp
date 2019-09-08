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
