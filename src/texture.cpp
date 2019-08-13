#include "texture.hpp"

Texture::Texture(RenderContext& context, const Bitmap& bitmap,
			uint32 internalPixelFormat)
		: context(&context)
		, textureID(-1)
		, width(bitmap.getWidth())
		, height(bitmap.getHeight()) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, internalPixelFormat,
			width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &textureID);
}
