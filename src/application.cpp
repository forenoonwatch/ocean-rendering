#include "application.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

uint32 Application::numInstances = 0;

Application::Application() {
	if (numInstances == 0) {
		glfwInit();
	}

	++numInstances;

	int32 monitorCount;
	GLFWmonitor** monitorHandles = glfwGetMonitors(&monitorCount);

	monitors = new Monitor[monitorCount];

	int32 xPos, yPos, width, height;

	for (uint32 i = 0; i < monitorCount; ++i) {
		glfwGetMonitorWorkarea(monitorHandles[i], &xPos,
				&yPos, &width, &height);
		
		monitors[i] = Monitor(monitorHandles[i], xPos, yPos,
				width, height);
	}
}

void Application::pollEvents() {
	glfwPollEvents();
}

Application::~Application() {
	--numInstances;

	if (numInstances == 0) {
		glfwTerminate();
	}

	delete[] monitors;
}
