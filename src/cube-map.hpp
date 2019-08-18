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
		RenderContext* context;
		uint32 textureID;

		CubeMap(RenderContext&);
};
