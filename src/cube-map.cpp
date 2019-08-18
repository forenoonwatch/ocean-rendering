#include "cube-map.hpp"

CubeMap::CubeMap(RenderContext& context, Bitmap* bitmaps)
		: CubeMap(context) {
	for (uint32 i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, bitmaps[i].getWidth(),
				bitmaps[i].getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmaps[i].getPixels());
	}
}

CubeMap::CubeMap(RenderContext& context, std::string* fileNames)
		: CubeMap(context) {
	Bitmap bmp;

	for (uint32 i = 0; i < 6; ++i) {
		bmp.load(fileNames[i]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, bmp.getWidth(),
				bmp.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.getPixels());
	}
}

inline CubeMap::CubeMap(RenderContext& context)
		: context(&context)
		, textureID(-1) {
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
