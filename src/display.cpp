#include "display.hpp"

int Display::numInstances = 0;

Display::Display(const char* title, int width, int height)
		: window(nullptr)
		, width(width)
		, height(height) {
	if (numInstances == 0) {
		glfwInit();
	}

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);

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
