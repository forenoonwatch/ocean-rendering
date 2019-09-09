#pragma once

#include "render-context.hpp"

#include "bitmap.hpp"
#include "dds-texture.hpp"

class CubeMap {
	public:
		CubeMap(RenderContext& context, Bitmap* bitmaps,
				uint32 internalFormat = GL_RGB);
		CubeMap(RenderContext& context, std::string* fileNames,
				uint32 internalFormat = GL_RGB);
		CubeMap(RenderContext& context, const DDSTexture& ddsTexture);

		inline uint32 getID() { return textureID; }

		inline uint32 getInternalFormat() const { return internalFormat; }

		inline bool isCompressed() const { return compressed; }
		inline bool hasMipMaps() const { return mipMaps; }

		~CubeMap();
	private:
		NULL_COPY_AND_ASSIGN(CubeMap);

		RenderContext* context;

		uint32 textureID;

		uint32 internalFormat;

		bool compressed;
		bool mipMaps;

		CubeMap(RenderContext&, uint32, bool, bool);
};
