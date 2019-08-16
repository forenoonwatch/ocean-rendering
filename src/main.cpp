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
#include "ocean-fft.hpp"

#define MOVE_SPEED	0.05f

void onKeyEvent(GLFWwindow*, int, int, int, int);
void onMouseClicked(GLFWwindow*, int, int, int);
void onMouseMoved(GLFWwindow*, double, double);

void updateCameraMovement(Display&);

Camera* camera;
bool lockCamera;
bool renderWater;
uint32 primitive;

int main() {
	Display display("MoIsT - Sponsored by Doritos(TM)", 800, 600);

	lockCamera = true;
	renderWater = true;
	primitive = GL_TRIANGLES;

	float fieldOfView = glm::radians(70.f);
	float aspectRatio = (float)display.getWidth() / (float)display.getHeight();
	float zNear = 0.1f;
	float zFar = 10.f;//100.f;

	Camera userCamera(fieldOfView, aspectRatio, zNear, 10.f * zFar);
	camera = &userCamera;

	Ocean ocean(0.f, 4.f, 256);

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

	fileData.str("");
	Util::resolveFileLinking(fileData, "./src/basic-compute.glsl", "#include");
	Shader computeShader(context, fileData.str());

	UniformBuffer dataBuffer(context, 4 * sizeof(glm::vec4), GL_DYNAMIC_DRAW);

	//basicShader.setUniformBuffer("ShaderData", dataBuffer, 0);
	oceanShader.setUniformBuffer("ShaderData", dataBuffer, 0);

	Sampler oceanSampler(context, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	Sampler sampler(context, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);

	OceanFFT oceanFFT(context, 256, 1000, false);
	//oceanFFT.init(4.f, glm::vec2(1.f, 1.f), 40.f, 0.5f);
	//oceanFFT.init(2.f, glm::vec2(1.f, 1.f), 80.f, 0.1f);
	oceanFFT.init(10.f, glm::vec2(1.f, 1.f), 80.f, 0.5f);
	context.awaitFinish();

	IndexedModel quadModel;
	quadModel.allocateElement(2);
	quadModel.setInstancedElementStartIndex(1);
	quadModel.allocateElement(2);

	quadModel.addElement2f(0, 0.f, 0.f);
	quadModel.addElement2f(0, 0.f, 1.f);
	quadModel.addElement2f(0, 1.f, 0.f);
	quadModel.addElement2f(0, 1.f, 1.f);

	quadModel.addIndices3i(2, 1, 0);
	quadModel.addIndices3i(1, 2, 3);

	VertexArray quad(context, quadModel, GL_STATIC_DRAW);

	Bitmap bmp;
	bmp.load("./res/foam.jpg");
	Texture foam(context, bmp, GL_RGBA);

	while (!display.isCloseRequested()) {
		updateCameraMovement(display);
		camera->update();

		if (lockCamera) {
			projector.update();
		}

		oceanFFT.update(1.f / 60.f);

		// BEGIN DRAW
		context.clear();

		dataBuffer.update(projector.getCorners(), 4 * sizeof(glm::vec4));

		oceanArray.updateBuffer(2, glm::value_ptr(camera->getViewProjection()),
				sizeof(glm::mat4));

		if (renderWater) {
			oceanShader.setSampler("ocean", oceanFFT.getDXYZ(), oceanSampler, 0);
			oceanShader.setSampler("foam", foam, oceanSampler, 1);
			context.draw(oceanShader, oceanArray, primitive);
		}
		else {
			basicShader.setSampler("diffuse", oceanFFT.getCoeffDX(), sampler, 0);
			quad.updateBuffer(1, glm::value_ptr(glm::vec2(-1.f, 0.f)), sizeof(glm::vec2));
			context.draw(basicShader, quad, primitive);

			basicShader.setSampler("diffuse", oceanFFT.getCoeffDY(), sampler, 0);
			quad.updateBuffer(1, glm::value_ptr(glm::vec2(0.f, 0.f)), sizeof(glm::vec2));
			context.draw(basicShader, quad, primitive);

			basicShader.setSampler("diffuse", oceanFFT.getDXYZ(), sampler, 0);
			quad.updateBuffer(1, glm::value_ptr(glm::vec2(-1.f, -1.f)), sizeof(glm::vec2));
			context.draw(basicShader, quad, primitive);

			basicShader.setSampler("diffuse", oceanFFT.getCoeffDZ(), sampler, 0);
			quad.updateBuffer(1, glm::value_ptr(glm::vec2(0.f, -1.f)), sizeof(glm::vec2));
			context.draw(basicShader, quad, primitive);
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

void updateCameraMovement(Display& display) {
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
}
