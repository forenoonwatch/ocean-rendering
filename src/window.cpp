#include "window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

Window::Window(Application& application,
			const char* title, uint32 width, uint32 height)
		: application(&application)
		, handle(nullptr)
		, width(width)
		, height(height)
		, fullscreen(false)
		, currentMonitor(nullptr) {
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

void Window::setFullscreen(bool fullscreen) {
	if (fullscreen == this->fullscreen) {
		return;
	}

	this->fullscreen = fullscreen;

	if (fullscreen) {
		currentMonitor = &application->getPrimaryMonitor();

		glfwSetWindowMonitor(handle, currentMonitor->getHandle(),
				currentMonitor->getX(), currentMonitor->getY(),
				currentMonitor->getWidth(), currentMonitor->getHeight(),
				GLFW_DONT_CARE);
	}
	else {
		currentMonitor = currentMonitor == nullptr ?
				&application->getPrimaryMonitor() : currentMonitor;

		glfwSetWindowMonitor(handle, nullptr,
				currentMonitor->getX() + currentMonitor->getWidth() / 2 - width / 2,
				currentMonitor->getY() + currentMonitor->getHeight() / 2 - height / 2,
				width, height, GLFW_DONT_CARE);
	}
}

void Window::setFullscreen(bool fullscreen, Monitor& monitor) {
	if (fullscreen == this->fullscreen
			&& &monitor == this->currentMonitor) {
		return;
	}

	this->fullscreen = fullscreen;
	currentMonitor = &monitor;

	if (fullscreen) {
		glfwSetWindowMonitor(handle, currentMonitor->getHandle(),
				currentMonitor->getX(), currentMonitor->getY(),
				currentMonitor->getWidth(), currentMonitor->getHeight(),
				GLFW_DONT_CARE);
	}
	else {
		glfwSetWindowMonitor(handle, nullptr,
				currentMonitor->getX() + currentMonitor->getWidth() / 2 - width / 2,
				currentMonitor->getY() + currentMonitor->getHeight() / 2 - height / 2,
				width, height, GLFW_DONT_CARE);
	}
}

Window::~Window() {
	glfwDestroyWindow(handle);
}
