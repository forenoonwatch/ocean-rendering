#include "cube-map.hpp"

CubeMap::CubeMap(RenderContext& context, Bitmap* bitmaps)
		: CubeMap(context, false, false) {
	for (uint32 i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, bitmaps[i].getWidth(),
				bitmaps[i].getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmaps[i].getPixels());
	}
}

CubeMap::CubeMap(RenderContext& context, std::string* fileNames)
		: CubeMap(context, false, false) {
	Bitmap bmp;

	for (uint32 i = 0; i < 6; ++i) {
		bmp.load(fileNames[i]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, bmp.getWidth(),
				bmp.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.getPixels());
	}
}

CubeMap::CubeMap(RenderContext& context, const DDSTexture& ddsTexture)
		: CubeMap(context, ddsTexture.isCompressed(),
				ddsTexture.getMipMapCount() > 1) {
	const uint32 internalFormat = ddsTexture.getInternalPixelFormat();
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
	
	for (uint32 i = 0; i < 6; ++i) {
		uint32 w = ddsTexture.getWidth();
		uint32 h = ddsTexture.getHeight();

		for (uint32 level = 0; level < ddsTexture.getMipMapCount()
				&& (w || h); ++level) {
			uint32 size = w * h * blockSize;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level,
					internalFormat, w, h, 0, GL_RGBA, dataType,
					ddsTexture.getData() + offset);

			offset += size;
			w /= 2;
			h /= 2;
		}
	}
}

inline CubeMap::CubeMap(RenderContext& context, bool compressed,
			bool mipMaps)
		: context(&context)
		, textureID(-1)
		, compressed(compressed)
		, mipMaps(mipMaps) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

CubeMap::~CubeMap() {
	glDeleteTextures(1, &textureID);
}
