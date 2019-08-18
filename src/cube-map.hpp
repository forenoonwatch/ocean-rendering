#pragma once

#include "bitmap.hpp"
#include "render-context.hpp"

class CubeMap {
	public:
		CubeMap(RenderContext& context, Bitmap* bitmaps);
		CubeMap(RenderContext& context, std::string* fileNames);

		inline uint32 getID() { return textureID; }

		~CubeMap();
	private:
		NULL_COPY_AND_ASSIGN(CubeMap);

		RenderContext* context;
		uint32 textureID;

		CubeMap(RenderContext&);
};
