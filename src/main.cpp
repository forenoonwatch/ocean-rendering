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

#define MOVE_SPEED	0.05f
#define GRID_LENGTH	16
#define GRID_SIZE	GRID_LENGTH * GRID_LENGTH
#define OCEAN_HEIGHT 0.f

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

void onKeyEvent(GLFWwindow*, int, int, int, int);
void onMouseClicked(GLFWwindow*, int, int, int);
void onMouseMoved(GLFWwindow*, double, double);

Camera* camera;
bool lockCamera;

int main() {
	Display display("MoIsT", 800, 600);

	lockCamera = true;

	float fieldOfView = glm::radians(70.f);
	float aspectRatio = (float)display.getWidth() / (float)display.getHeight();
	float zNear = 0.1f;
	float zFar = 10.f;//100.f;

	Camera userCamera(fieldOfView, aspectRatio, zNear, 10.f * zFar);
	camera = &userCamera;

	Camera projectorCamera(fieldOfView, aspectRatio, zNear, zFar);

	glfwSetKeyCallback(display.getWindow(), onKeyEvent);
	glfwSetMouseButtonCallback(display.getWindow(), onMouseClicked);
	glfwSetCursorPosCallback(display.getWindow(), onMouseMoved);

	glm::vec2 vertices[GRID_SIZE];

	for (int y = 0; y < GRID_LENGTH; ++y) {
		for (int x = 0; x < GRID_LENGTH; ++x) {
			int i = y * GRID_LENGTH + x;
			vertices[i] = glm::vec2((float)x / (float)GRID_LENGTH,
					(float)y / (float)GRID_LENGTH);
		}
	}

	glm::vec3 cameraCube[] = {glm::vec3(-1, -1, -1), glm::vec3(1, -1, -1),
		glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, -1, 1),
		glm::vec3(1, -1, 1), glm::vec3(-1, 1, 1), glm::vec3(1, 1, 1)};

	glm::vec4 frustum[8];

	int cameraCubeEdges[] = {0, 1,  0, 2,  2, 3,  1, 3,
							 0, 4,  2, 6,  3, 7,  1, 5,
							 4, 6,  4, 5,  5, 7,  6, 7};

	glm::vec3 sects[12];
	int numSects;

	glm::vec4 transformedVerts[GRID_SIZE];

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
			projectorCamera.update(*camera);

			for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(frustum); ++i) {
				frustum[i] = projectorCamera.getInverseVP() * glm::vec4(cameraCube[i], 1.f);
				frustum[i] /= frustum[i].w;
			}

			numSects = 0;
			
			float minX = FLT_MAX, maxX = -FLT_MAX;
			float minY = FLT_MAX, maxY = -FLT_MAX;

			for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(cameraCubeEdges); i += 2) {
				glm::vec3 a = glm::vec3(frustum[cameraCubeEdges[i]]);
				glm::vec3 b = glm::vec3(frustum[cameraCubeEdges[i + 1]]);
				glm::vec3 v = b - a;
				float m = glm::length2(v);

				float t = (OCEAN_HEIGHT - a.y) / v.y;

				if (t >= 0.f && t <= m) {
					glm::vec4 p = projectorCamera.getViewProjection() * glm::vec4(a + v * t, 1.f);
					p /= p.w;

					if (p.x < minX) {
						minX = p.x;
					}

					if (p.x > maxX) {
						maxX = p.x;
					}

					if (p.y < minY) {
						minY = p.y;
					}

					if (p.y > maxY) {
						maxY = p.y;
					}

					sects[numSects++] = a + v * t;
				}
			}

			glm::mat4 mRange(1.f);

			mRange[0][0] = maxX - minX;
			mRange[1][1] = maxY - minY;
			mRange[3][0] = minX;
			mRange[3][1] = minY;

			glm::mat4 mProjector = projectorCamera.getInverseVP() * mRange;

			for (int i = 0; i < GRID_SIZE; ++i) {
				glm::vec2& vert = vertices[i];

				glm::vec4 a = mProjector * glm::vec4(vert.x, vert.y, 1.f, 1.f);
				glm::vec4 b = mProjector * glm::vec4(vert.x, vert.y, -1.f, 1.f);

				a /= a.w;
				b /= b.w;

				glm::vec3 v = glm::vec3(b) - glm::vec3(a);
				float t = (OCEAN_HEIGHT - a.y) / v.y;
				glm::vec3 p = glm::vec3(a) + v * t;
				p.y = std::sin(p.x);

				transformedVerts[i] = glm::vec4(p, 1.f);
			}

			//printf("X(%.2f, %.2f)\n", minX, maxX);
			//printf("Y(%.2f, %.2f)\n", minY, maxY);
			//printf("%d intersections\n", numSects);
		}

		//glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.f));
		//model *= glm::rotate(glm::mat4(1.f), (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 model = projectorCamera.getInverseVP();
		glm::mat4 mvp = camera->getViewProjection()/* * model*/;

		//glLoadMatrixf(glm::value_ptr(mvp));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBegin(GL_LINES);
		glColor3f(1.f, 1.f, 1.f);

		for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(cameraCubeEdges); i += 2) {
			glVertex4fv(glm::value_ptr(camera->getViewProjection() * frustum[cameraCubeEdges[i]]));
			glVertex4fv(glm::value_ptr(camera->getViewProjection() * frustum[cameraCubeEdges[i + 1]]));
		}

		glEnd();

		/*glBegin(GL_LINE_LOOP);
		//glPointSize(5.f);
		glColor3f(1.f, 0.f, 0.f);

		for (int i = 0; i < numSects; ++i) {
			glVertex4fv(glm::value_ptr(camera->getViewProjection() * glm::vec4(sects[i].x,
							sects[i].y, sects[i].z, 1.f)));
		}

		glEnd();*/

		glBegin(GL_POINTS);
		glColor3f(1.f, 0.f, 0.f);

		for (int i = 0; i < GRID_SIZE; ++i) {
			glVertex4fv(glm::value_ptr(camera->getViewProjection() * transformedVerts[i]));
		}

		glEnd();

		display.render();
		display.pollEvents();
	}

	return 0;
}

void onKeyEvent(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		lockCamera = !lockCamera;
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
