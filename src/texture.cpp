#include "texture.hpp"

Texture::Texture(RenderContext& context, uint32 width,
			uint32 height, uint32 internalPixelFormat, bool storage,
			const void* data, uint32 pixelFormat, uint32 dataType)
		: context(&context)
		, textureID(-1)
		, width(width)
		, height(height)
		, internalFormat(internalPixelFormat) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (storage) {
		glTexStorage2D(GL_TEXTURE_2D, 0,
				internalPixelFormat, width, height);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, internalPixelFormat,
				width, height, 0, pixelFormat, dataType, data);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

Texture::Texture(RenderContext& context, const Bitmap& bitmap,
			uint32 internalPixelFormat)
		: Texture(context, bitmap.getWidth(), bitmap.getHeight(),
			   internalPixelFormat, false, bitmap.getPixels())	{}

Texture::~Texture() {
	glDeleteTextures(1, &textureID);
}
