#pragma once

#include "common.hpp"

class GLFWmonitor;

typedef GLFWmonitor* MonitorHandle;

class Monitor {
	public:
		Monitor(MonitorHandle handle = nullptr, uint32 x = 0,
				uint32 y = 0, uint32 width = 0, uint32 height = 0);

		inline MonitorHandle getHandle() { return handle; }

		inline uint32 getX() const { return x; }
		inline uint32 getY() const { return y; }

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		~Monitor();
	private:
		MonitorHandle handle;

		uint32 x;
		uint32 y;
		uint32 width;
		uint32 height;
};
