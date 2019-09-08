#include "dds-texture.hpp"

#include <cstdio>
#include <cstring>

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
	fourCC = *((uint32*)&header[80]);

	cubeMap = false; // TODO: implement cubemap support

	uint32 dataSize = mipMapCount > 1 ? linearSize * 2 : linearSize;

	cleanUp();
	data = new uint8[dataSize];
	fread(data, 1, dataSize, file);

	fclose(file);
	return true;
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
