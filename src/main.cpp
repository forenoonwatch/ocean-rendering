#include <cmath>
#include <cstdio>
#include <algorithm>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "display.hpp"
#include "camera.hpp"

#include "vertex-array.hpp"
#include "shader.hpp"
#include "util.hpp"

#include "ocean.hpp"

#define MOVE_SPEED	0.05f

void onKeyEvent(GLFWwindow*, int, int, int, int);
void onMouseClicked(GLFWwindow*, int, int, int);
void onMouseMoved(GLFWwindow*, double, double);

Camera* camera;
bool lockCamera;
bool renderWater;
uint32 primitive;

int main() {
	Display display("MoIsT", 800, 600);

	lockCamera = true;
	renderWater = true;
	primitive = GL_TRIANGLES;

	float fieldOfView = glm::radians(70.f);
	float aspectRatio = (float)display.getWidth() / (float)display.getHeight();
	float zNear = 0.1f;
	float zFar = 10.f;//100.f;

	Camera userCamera(fieldOfView, aspectRatio, zNear, 10.f * zFar);
	camera = &userCamera;

	Ocean ocean(0.f, 1.f, 256);

	OceanProjector projector(ocean, userCamera);

	glfwSetKeyCallback(display.getWindow(), onKeyEvent);
	glfwSetMouseButtonCallback(display.getWindow(), onMouseClicked);
	glfwSetCursorPosCallback(display.getWindow(), onMouseMoved);

	RenderContext context;
	VertexArray oceanArray(context, ocean, GL_STATIC_DRAW);

	std::stringstream fileData;
	Util::resolveFileLinking(fileData, "./src/basic-shader.glsl", "#include");
	Shader basicShader(context, fileData.str());
	
	fileData.str("");
	Util::resolveFileLinking(fileData, "./src/ocean-shader.glsl", "#include");
	Shader oceanShader(context, fileData.str());

	UniformBuffer dataBuffer(context, 4 * sizeof(glm::vec4), GL_DYNAMIC_DRAW);

	basicShader.setUniformBuffer("ShaderData", dataBuffer, 0);

	IndexedModel testModel;
	testModel.allocateElement(4);
	testModel.allocateElement(16);
	testModel.setInstancedElementStartIndex(1);

	testModel.addElement4f(0, -1.f, -1.f, 0.f, 1.f);
	testModel.addElement4f(0, -1.f,  1.f, 0.f, 1.f);
	testModel.addElement4f(0,  1.f, -1.f, 0.f, 1.f);
	testModel.addElement4f(0,  1.f,  1.f, 0.f, 1.f);

	testModel.addIndices3i(0, 1, 2);
	testModel.addIndices3i(1, 2, 3);

	VertexArray testArray(context, testModel, GL_DYNAMIC_DRAW);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	while (!display.isCloseRequested()) {
		float dx = 0.f, dy = 0.f, dz = 0.f;

		if (glfwGetKey(display.getWindow(), GLFW_KEY_W) == GLFW_PRESS) {
			dz -= MOVE_SPEED;
		}
		
		if (glfwGetKey(display.getWindow(), GLFW_KEY_S) == GLFW_PRESS) {
			dz += MOVE_SPEED;
		}

		if (glfwGetKey(display.getWindow(), GLFW_KEY_A) == GLFW_PRESS) {
			dx -= MOVE_SPEED;
		}

		if (glfwGetKey(display.getWindow(), GLFW_KEY_D) == GLFW_PRESS) {
			dx += MOVE_SPEED;
		}

		if (glfwGetKey(display.getWindow(), GLFW_KEY_Q) == GLFW_PRESS) {
			dy -= MOVE_SPEED;
		}
		
		if (glfwGetKey(display.getWindow(), GLFW_KEY_E) == GLFW_PRESS) {
			dy += MOVE_SPEED;
		}

		camera->move(dx, dy, dz);
		camera->update();

		if (lockCamera) {
			projector.update();
			testArray.updateBuffer(0, projector.getCorners(), 4 * sizeof(glm::vec4));
		}

		//glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.f));
		//model *= glm::rotate(glm::mat4(1.f), (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
		//glm::mat4 model = projectorCamera.getInverseVP();
		//glm::mat4 mvp = camera->getViewProjection() * projector.getProjectorMatrix()/* * model*/;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//dataBuffer.update(glm::value_ptr(camera->getPosition()), sizeof(glm::vec3));
		dataBuffer.update(projector.getCorners(), 4 * sizeof(glm::vec4));

		testArray.updateBuffer(1, glm::value_ptr(camera->getViewProjection()), sizeof(glm::mat4));
		
		glm::mat4 transforms[] = {camera->getViewProjection(), projector.getProjectorMatrix()};
		oceanArray.updateBuffer(2, transforms, 2 * sizeof(glm::mat4));

		if (renderWater) {
			glUseProgram(oceanShader.getID());
			oceanArray.draw(primitive, 1);
		}
		else {
			//glUseProgram(basicShader.getID());
			//testArray.draw(primitive, 1);
			glUseProgram(0);
			
			glBegin(GL_POINTS);
			for (float y = 0.f; y <= 1.f; y += 0.1f) {
				for (float x = 0.f; x <= 1.f; x += 0.1f) {
					const glm::vec4 a = glm::mix(projector.getCorners()[0],
							projector.getCorners()[2], x);
					const glm::vec4 b = glm::mix(projector.getCorners()[1],
							projector.getCorners()[3], x);
					const glm::vec4 c = glm::mix(a, b, y);

					glVertex4fv(glm::value_ptr(camera->getViewProjection() * c));
				}
			}
			glEnd();
		}

		display.render();
		display.pollEvents();
	}

	return 0;
}

void onKeyEvent(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_R:
				lockCamera = !lockCamera;
				break;
			case GLFW_KEY_F:
				renderWater = !renderWater;
				break;
			case GLFW_KEY_G:
				primitive = primitive == GL_TRIANGLES ? GL_LINES : GL_TRIANGLES;
				break;
		}
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
	static double lastX = 0.0;
	static double lastY = 0.0;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		camera->rotate((float)( (lastY - yPos) * 0.01 ), (float)( (lastX - xPos) * 0.01 ));
	}

	lastX = xPos;
	lastY = yPos;
}

