#pragma once

#include "common.hpp"

#include <string>

class Bitmap {
	public:
		Bitmap(int32 width = 1, int32 height = 1);
		Bitmap(int32 width, int32 height, int32* pixels);

		void clear();

		bool load(const std::string& fileName);

		inline void set(int32 x, int32 y, int32 abgr);

		inline int32 getWidth() const { return width; }
		inline int32 getHeight() const { return height; }

		inline int32* getPixels() { return pixels; }
		inline const int32* getPixels() const { return pixels; }

		~Bitmap();
	private:
		NULL_COPY_AND_ASSIGN(Bitmap);

		int32 width;
		int32 height;
		int32* pixels;

		uintptr calcPixelsSize() const;
};

inline void Bitmap::set(int32 x, int32 y, int32 abgr) {
	pixels[y * width + x] = abgr;
}
