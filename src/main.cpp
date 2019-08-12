#include <cmath>
#include <cstdio>
#include <algorithm>
#include <cfloat>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtx/norm.hpp>

#include "display.hpp"
#include "camera.hpp"

#include "vertex-array.hpp"
#include "shader.hpp"

#include "util.hpp"

#define MOVE_SPEED	0.05f
#define GRID_LENGTH	16
#define GRID_SIZE	GRID_LENGTH * GRID_LENGTH
#define OCEAN_HEIGHT 0.f

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

void onKeyEvent(GLFWwindow*, int, int, int, int);
void onMouseClicked(GLFWwindow*, int, int, int);
void onMouseMoved(GLFWwindow*, double, double);

int main() {
	Display display("MoIsT", 800, 600);

	glfwSetKeyCallback(display.getWindow(), onKeyEvent);
	glfwSetMouseButtonCallback(display.getWindow(), onMouseClicked);
	glfwSetCursorPosCallback(display.getWindow(), onMouseMoved);

	RenderContext renderContext;
	IndexedModel model;

	model.allocateElement(2);

	model.addElement2f(0, -1.f, -1.f);
	model.addElement2f(0,  0.f,  1.f);
	model.addElement2f(0,  1.f, -1.f);

	model.addIndices3i(0, 1, 2);

	VertexArray ary(renderContext, model, GL_STATIC_DRAW);

	std::stringstream ss;
	Util::resolveFileLinking(ss, "./src/basic-shader.glsl", "#include");

	Shader shader(renderContext, ss.str());

	while (!display.isCloseRequested()) {
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader.getID());
		ary.draw(GL_TRIANGLES, 1);

		display.render();
		display.pollEvents();
	}

	return 0;
}

void onKeyEvent(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		//lockCamera = !lockCamera;
	}
}

void onMouseClicked(GLFWwindow* window, int button, int action, int mods) {
	/*if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			std::cout << "Pressed RMB" << std::endl;
		}
		else {
			std::cout << "Released RMB" << std::endl;
		}
	}*/
}

void onMouseMoved(GLFWwindow* window, double xPos, double yPos) {
	/*static double lastX = 0.0;
	static double lastY = 0.0;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		camera->rotate((float)( (lastY - yPos) * 0.01 ), (float)( (lastX - xPos) * 0.01 ));
	}

	lastX = xPos;
	lastY = yPos;*/
}
