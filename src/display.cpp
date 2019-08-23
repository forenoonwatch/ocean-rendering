#include "display.hpp"

uint32 Display::numInstances = 0;

Display::Display(const char* title, uint32 width, uint32 height)
		: window(nullptr)
		, width(width)
		, height(height) {
	if (numInstances == 0) {
		glfwInit();
	}

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glewInit();

	++numInstances;
}

bool Display::isCloseRequested() const {
	return glfwWindowShouldClose(window);
}

void Display::pollEvents() {
	glfwPollEvents();
}

void Display::render() {
	glfwSwapBuffers(window);
}

Display::~Display() {
	--numInstances;

	glfwDestroyWindow(window);

	if (numInstances == 0) {
		glfwTerminate();
	}
}
