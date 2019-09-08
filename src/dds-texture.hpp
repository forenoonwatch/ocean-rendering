#pragma once

#include "common.hpp"

#include <string>

#define MAKEFOURCC(a, b, c, d)                              \
                ((uint32)(uint8)(a) | ((uint32)(uint8)(b) << 8) |       \
				((uint32)(uint8)(c) << 16) | ((uint32)(uint8)(d) << 24 ))

#define MAKEFOURCCDXT(a) MAKEFOURCC('D', 'X', 'T', a)

#define FOURCC_DXT1 MAKEFOURCCDXT('1')
#define FOURCC_DXT2 MAKEFOURCCDXT('2')
#define FOURCC_DXT3 MAKEFOURCCDXT('3')
#define FOURCC_DXT4 MAKEFOURCCDXT('4')
#define FOURCC_DXT5 MAKEFOURCCDXT('5')

class DDSTexture {
	public:
		inline DDSTexture()
				: data(nullptr) {}

		bool load(const std::string& fileName);

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline uint32 getMipMapCount() const { return mipMapCount; }
		inline uint32 getFourCC() const { return fourCC; }

		inline bool isCubeMap() const { return cubeMap; }

		inline const uint8* getData() const { return data; }

		~DDSTexture();
	private:
		NULL_COPY_AND_ASSIGN(DDSTexture);

		uint32 width;
		uint32 height;

		uint32 mipMapCount;
		uint32 fourCC;

		bool cubeMap;

		uint8* data;

		void cleanUp();
};
