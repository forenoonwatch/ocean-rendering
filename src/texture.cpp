#include "texture.hpp"

static uint32 calcInternalFormat(uint32 pixelFormat, bool compressed);
static uint32 calcDDSInternalFormat(uint32 fourCC);

static bool isDDSCompressed(uint32 fourCC);

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
		, internalFormat(calcDDSInternalFormat(ddsTexture.getFourCC()))
		, compressed(isDDSCompressed(ddsTexture.getFourCC()))
		, mipMaps(ddsTexture.getMipMapCount() > 1) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	const uint32 blockSize = (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	uint32 offset = 0;

	for (uint32 level = 0; level < ddsTexture.getMipMapCount()
			&& (width || height); ++level) {
		uint32 size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;

		if (compressed) {
			glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat,
					width, height, 0, size, ddsTexture.getData() + offset);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, level, internalFormat,
					width, height, 0, GL_RGBA32F, GL_FLOAT, ddsTexture.getData() + offset);
		}

		offset += size;
		width /= 2;
		height /= 2;
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

static uint32 calcDDSInternalFormat(uint32 fourCC) {
	switch (fourCC) {
		case FOURCC_DXT1:
			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		case FOURCC_DXT3:
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		case FOURCC_DXT5:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case FOURCC_A16B16G16R16F:
			return GL_RGBA16F;
		case FOURCC_A32B32G32R32F:
			return GL_RGBA32F;
		default:
			DEBUG_LOG(LOG_ERROR, "Texture",
					"%s is not a valid DDS texture compression format",
					(const char*)(&fourCC));
			return 0;
	}
}

static bool isDDSCompressed(uint32 fourCC) {
	switch (fourCC) {
		case FOURCC_DXT1:
		case FOURCC_DXT3:
		case FOURCC_DXT5:
			return true;
		default:
			return false;
	}
}
