#include "application.hpp"

#include <cstring>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

Monitor* Application::monitors = nullptr;

bool Application::keys[] = {0};
bool Application::mouseButtons[] = {0};

bool Application::lastKeys[] = {0};
bool Application::lastMouseButtons[] = {0};

double Application::mouseX = 0.0;
double Application::mouseY = 0.0;

void Application::init() {
	glfwInit();

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
	std::memcpy(lastKeys, keys, sizeof(keys));
	std::memcpy(lastMouseButtons, mouseButtons, sizeof(mouseButtons));
	
	glfwPollEvents();
}

bool Application::isKeyDown(enum Input::KeyCode keyCode) {
	return keys[keyCode];
}

bool Application::getKeyPressed(enum Input::KeyCode keyCode) {
	return keys[keyCode] && !lastKeys[keyCode];
}

bool Application::getKeyReleased(enum Input::KeyCode keyCode) {
	return !keys[keyCode] && lastKeys[keyCode];
}

bool Application::isMouseDown(enum Input::MouseButton mouseButton) {
	return mouseButtons[mouseButton];
}

bool Application::getMousePressed(enum Input::MouseButton mouseButton) {
	return mouseButtons[mouseButton] && !lastMouseButtons[mouseButton];
}

bool Application::getMouseReleased(enum Input::MouseButton mouseButton) {
	return !mouseButtons[mouseButton] && lastMouseButtons[mouseButton];
}

void Application::destroy() {
	glfwTerminate();

	delete[] monitors;
}

void Application::bindInputCallbacks(WindowHandle windowHandle) {
	glfwSetKeyCallback(windowHandle, Application::onKeyEvent);
	glfwSetMouseButtonCallback(windowHandle, Application::onMouseClickEvent);
	glfwSetCursorPosCallback(windowHandle, Application::onMouseMoveEvent);
}

void Application::onKeyEvent(WindowHandle window, int key,
		int scanCode, int action, int modifiers) {
	keys[key] = action;
}

void Application::onMouseClickEvent(WindowHandle window,
		int button, int action, int modifiers) {
	mouseButtons[button] = action;
}

void Application::onMouseMoveEvent(WindowHandle window,
		double xPos, double yPos) {
	Application::mouseX = xPos;
	Application::mouseY = yPos;
}
