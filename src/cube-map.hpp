#pragma once

#include "render-context.hpp"

#include "bitmap.hpp"
#include "dds-texture.hpp"

class CubeMap {
	public:
		CubeMap(RenderContext& context, Bitmap* bitmaps);
		CubeMap(RenderContext& context, std::string* fileNames);
		CubeMap(RenderContext& context, const DDSTexture& ddsTexture);

		inline uint32 getID() { return textureID; }

		inline bool isCompressed() const { return compressed; }
		inline bool hasMipMaps() const { return mipMaps; }

		~CubeMap();
	private:
		NULL_COPY_AND_ASSIGN(CubeMap);

		RenderContext* context;

		uint32 textureID;

		bool compressed;
		bool mipMaps;

		CubeMap(RenderContext&, bool, bool);
};
