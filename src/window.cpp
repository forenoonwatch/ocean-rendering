#include "window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

Window::Window(const Application& application,
			const char* title, uint32 width, uint32 height)
		: handle(nullptr)
		, width(width)
		, height(height) {
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(handle);

	glewInit();
}

bool Window::isCloseRequested() const {
	return glfwWindowShouldClose(handle);
}

void Window::render() {
	glfwSwapBuffers(handle);
}

Window::~Window() {
	glfwDestroyWindow(handle);
}
