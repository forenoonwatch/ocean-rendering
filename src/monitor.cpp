#include "monitor.hpp"

#include <GLFW/glfw3.h>

Monitor::Monitor(MonitorHandle handle, uint32 x,
			uint32 y, uint32 width, uint32 height)
		: handle(handle)
		, x(x)
		, y(y)
		, width(width) 
		, height(height) {}

Monitor::~Monitor() {}
