#include "application.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

uint32 Application::numInstances = 0;

Application::Application() {
	if (numInstances == 0) {
		glfwInit();
	}

	++numInstances;
}

void Application::pollEvents() {
	glfwPollEvents();
}

Application::~Application() {
	--numInstances;

	if (numInstances == 0) {
		glfwTerminate();
	}
}
