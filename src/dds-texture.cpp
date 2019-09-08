#include "dds-texture.hpp"

#include <GL/glew.h>

#include <cstdio>
#include <cstring>

// caps1
#define DDSCAPS_COMPLEX             0x00000008 
#define DDSCAPS_TEXTURE             0x00001000 
#define DDSCAPS_MIPMAP              0x00400000 

// caps2
#define DDSCAPS2_CUBEMAP            0x00000200 
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400 
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800 
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000 
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000 
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000 
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000 
#define DDSCAPS2_VOLUME             0x00200000 

bool DDSTexture::load(const std::string& fileName) {
	FILE* file = fopen(fileName.c_str(), "rb");

	if (file == nullptr) {
		DEBUG_LOG(LOG_ERROR, "DDS Texture",
				"Failed to open DDS Texture: %s", fileName.c_str());
		return false;
	}

	char magic[4];
	fread(magic, 1, 4, file);

	if (strncmp(magic, "DDS ", 4) != 0) {
		DEBUG_LOG(LOG_ERROR, "DDS Texture",
				"File %s is not a valid DDS Texture", fileName.c_str());

		fclose(file);
		return false;
	}

	uint8 header[124];
	fread(&header, ARRAY_SIZE_IN_ELEMENTS(header), 1, file);

	height = *((uint32*)&header[8]);
	width = *((uint32*)&header[12]);
	uint32 linearSize = *((uint32*)&header[16]);

	mipMapCount = *((uint32*)&header[24]);

	uint32 flags = *((uint32*)&header[76]);
	fourCC = *((uint32*)&header[80]);

	//uint32 caps1 = *((uint32*)&header[104]);
	uint32 caps2 = *((uint32*)&header[108]);

	cubeMap = caps2 & DDSCAPS2_CUBEMAP;

	uint32 dataSize = mipMapCount > 1 ? linearSize * 2 : linearSize;

	cleanUp();
	data = new uint8[dataSize];
	fread(data, 1, dataSize, file);

	fclose(file);
	return true;
}

uint32 DDSTexture::getInternalPixelFormat() const {
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

bool DDSTexture::isCompressed() const {
	switch (fourCC) {
		case FOURCC_DXT1:
		case FOURCC_DXT3:
		case FOURCC_DXT5:
			return true;
		default:
			return false;
	}
}

DDSTexture::~DDSTexture() {
	cleanUp();
}

inline void DDSTexture::cleanUp() {
	if (data != nullptr) {
		delete[] data;
		data = nullptr;
	}
}
