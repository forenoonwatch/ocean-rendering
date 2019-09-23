#pragma once

#include "input.hpp"
#include "monitor.hpp"

class GLFWwindow;

typedef GLFWwindow* WindowHandle;

class Window {
	public:
		Window(const char* title, uint32 width, uint32 height);

		bool isCloseRequested() const;
		void render();

		void setFullscreen(bool fullscreen);
		void setFullscreen(bool fullscreen, Monitor& monitor);

		void setCursorMode(enum Input::CursorMode cursorMode);

		inline WindowHandle getHandle() { return handle; }
		
		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline bool isFullscreen() const { return fullscreen; }

		~Window();
	private:
		NULL_COPY_AND_ASSIGN(Window);

		WindowHandle handle;
		uint32 width;
		uint32 height;

		bool fullscreen;
		Monitor* currentMonitor;
};
