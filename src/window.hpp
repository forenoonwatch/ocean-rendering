#pragma once

#include "application.hpp"

class GLFWwindow;

typedef GLFWwindow* WindowHandle;

class Window {
	public:
		Window(const Application& application, const char* title,
				uint32 width, uint32 height);

		bool isCloseRequested() const;
		void render();

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline WindowHandle getHandle() { return handle; }

		~Window();
	private:
		NULL_COPY_AND_ASSIGN(Window);

		WindowHandle handle;
		uint32 width;
		uint32 height;
};
