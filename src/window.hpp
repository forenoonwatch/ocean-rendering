#pragma once

#include "application.hpp"

class GLFWwindow;

typedef GLFWwindow* WindowHandle;

class Window {
	public:
		Window(Application& application, const char* title,
				uint32 width, uint32 height);

		bool isCloseRequested() const;
		void render();

		void setFullscreen(bool fullscreen);
		void setFullscreen(bool fullscreen, Monitor& monitor);

		inline WindowHandle getHandle() { return handle; }
		
		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline bool isFullscreen() const { return fullscreen; }

		~Window();
	private:
		NULL_COPY_AND_ASSIGN(Window);

		Application* application;

		WindowHandle handle;
		uint32 width;
		uint32 height;

		bool fullscreen;
		Monitor* currentMonitor;
};
