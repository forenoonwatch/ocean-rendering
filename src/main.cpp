#include <cmath>
#include <cstdio>
#include <algorithm>

#include <memory>
#include <unordered_map>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "display.hpp"
#include "camera.hpp"

#include "render-target.hpp"
#include "vertex-array.hpp"
#include "shader.hpp"
#include "util.hpp"

#include "ocean.hpp"
#include "ocean-fft.hpp"
#include "ocean-projector.hpp"

#include "gaussian-blur.hpp"

#define MOVE_SPEED	0.05f

void onKeyEvent(GLFWwindow*, int, int, int, int);
void onMouseClicked(GLFWwindow*, int, int, int);
void onMouseMoved(GLFWwindow*, double, double);

void updateCameraMovement(Display&);

void createCube(IndexedModel&);
void loadShaders(RenderContext&,
		std::unordered_map<std::string, std::shared_ptr<Shader>>&);

void setBeaufortLevel(OceanFFT&, UniformBuffer&, const glm::vec2&, float);

Camera* camera;
bool lockCamera;
bool renderWater;
uint32 primitive;

float beaufort;

int main() {
	Display display("MoIsT - Sponsored by Doritos(TM)", 1200, 900);

	lockCamera = true;
	renderWater = true;
	primitive = GL_TRIANGLES;

	beaufort = 0.f;
	float lastBeaufort = 0.f;

	float fieldOfView = glm::radians(70.f);
	float aspectRatio = (float)display.getWidth() / (float)display.getHeight();
	float zNear = 0.1f;
	float zFar = 100.f;

	Camera userCamera(fieldOfView, aspectRatio, zNear, 10.f * zFar);
	camera = &userCamera;

	glfwSetKeyCallback(display.getWindow(), onKeyEvent);
	glfwSetMouseButtonCallback(display.getWindow(), onMouseClicked);
	glfwSetCursorPosCallback(display.getWindow(), onMouseMoved);

	RenderContext context;
	std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

	loadShaders(context, shaders);
	
	Ocean ocean(context, 0.f, 4.f, 256);
	OceanProjector projector(ocean, userCamera);

	UniformBuffer oceanDataBuffer(context, 4 * sizeof(glm::vec4) + sizeof(glm::vec3)
			+ 3 * sizeof(float), GL_DYNAMIC_DRAW, 0);
	UniformBuffer lightDataBuffer(context, 1 * sizeof(glm::vec3)
			+ 3 * sizeof(float) + sizeof(glm::vec3) + 2 * sizeof(float), GL_DYNAMIC_DRAW, 1);

	shaders["ocean-shader"]->setUniformBuffer("OceanData", oceanDataBuffer);
	shaders["ocean-shader"]->setUniformBuffer("LightingData", lightDataBuffer);
	shaders["skybox-shader"]->setUniformBuffer("LightingData", lightDataBuffer);

	{
		float lightData[] = {0.2f, 15.f, 128.f};
		lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(1, 1, 1))), sizeof(glm::vec3));
		lightDataBuffer.update(lightData, sizeof(glm::vec3), sizeof(lightData));

		float fogData[] = {202.f / 255.f, 243.f / 255.f, 246.f / 255.f,
			0.002f, 1.f};

		lightDataBuffer.update(fogData, sizeof(glm::vec3) + sizeof(lightData)
				+ 2 * sizeof(float), sizeof(fogData));
	}

	Sampler oceanSampler(context, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	Sampler sampler(context, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
	Sampler skyboxSampler(context, GL_LINEAR, GL_LINEAR);

	OceanFFT oceanFFT(context, 256, 1000, true, 5.f);
	//oceanFFT.init(4.f, glm::vec2(1.f, 1.f), 40.f, 0.5f);
	//oceanFFT.setOceanParams(10.f, glm::vec2(1.f, 1.f), 80.f, 0.5f);
	context.awaitFinish();

	Bitmap bmp;
	bmp.load("./res/foam.jpg");
	Texture foam(context, bmp, GL_RGBA);

	bmp.load("./res/water-dudv.png");
	Texture oceanDUDV(context, bmp, GL_RGBA);

	IndexedModel cubeModel;
	createCube(cubeModel);

	VertexArray cube(context, cubeModel, GL_STATIC_DRAW);

	std::string cubeTextures[] = {"./res/sargasso_sea/right.tga",
		"./res/sargasso_sea/left.tga", "./res/sargasso_sea/top.tga",
		"./res/sargasso_sea/bottom.tga", "./res/sargasso_sea/front.tga",
		"./res/sargasso_sea/back.tga"};

	CubeMap skybox(context, cubeTextures);

	RenderTarget screen(context, display.getWidth(), display.getHeight());

	Texture reflection(context, display.getWidth(), 
			display.getHeight(), GL_RGBA);
	Texture hdrTexture(context, display.getWidth(),
			display.getHeight(), GL_RGBA32F);
	Texture brightTexture(context, display.getWidth(),
			display.getHeight(), GL_RGBA32F);
	
	RenderBuffer hdrDepthStencil(context, display.getWidth(),
			display.getHeight(), GL_DEPTH24_STENCIL8);

	RenderTarget reflectionTarget(context, reflection, GL_COLOR_ATTACHMENT0);
	RenderTarget hdrTarget(context, hdrTexture, GL_COLOR_ATTACHMENT0);

	hdrTarget.addRenderBuffer(hdrDepthStencil, GL_DEPTH_STENCIL_ATTACHMENT);
	hdrTarget.addTextureTarget(brightTexture, GL_COLOR_ATTACHMENT0, 1);

	GaussianBlur blurBuffer(context, *shaders["gaussian-blur-shader"], brightTexture);

	setBeaufortLevel(oceanFFT, oceanDataBuffer, glm::vec2(1, 1), beaufort);

	while (!display.isCloseRequested()) {
		updateCameraMovement(display);
		camera->update();

		if (lockCamera) {
			projector.update();
		}

		//oceanFFT.setOceanParams(10.f, glm::vec2(1, 1), 40.f, 100.f * std::sin(0.1 * glfwGetTime()));
		//context.awaitFinish();
		
		//if (beaufort != lastBeaufort) {
		//	setBeaufortLevel(oceanFFT, oceanDataBuffer, glm::vec2(1, 1), beaufort);
		//}

		//lastBeaufort = beaufort;

		setBeaufortLevel(oceanFFT, oceanDataBuffer, glm::vec2(1, 1),
				6.f + 6.f * std::sin(0.1 * glfwGetTime()));

		//lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(std::cos(0.2 * glfwGetTime()),
		//		std::sin(0.2 * glfwGetTime()), 0.f))),
		//		sizeof(glm::vec3));

		oceanFFT.update(1.f / 60.f);

		// BEGIN DRAW
		screen.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		oceanDataBuffer.update(projector.getCorners(), 4 * sizeof(glm::vec4));
		oceanDataBuffer.update(glm::value_ptr(camera->getPosition()),
				4 * sizeof(glm::vec4), sizeof(glm::vec3));

		ocean.getGridArray().updateBuffer(1, glm::value_ptr(camera->getViewProjection()),
				sizeof(glm::mat4));

		reflectionTarget.clear(GL_COLOR_BUFFER_BIT);

		cube.updateBuffer(1, glm::value_ptr(camera->getReflectionSkybox()),
				sizeof(glm::mat4));
		shaders["skybox-shader"]->setSampler("skybox", skybox, skyboxSampler, 0);
		context.draw(reflectionTarget, *shaders["skybox-shader"], cube, GL_TRIANGLES);

		hdrTarget.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		context.setDrawBuffers(2);

		shaders["ocean-shader"]->setSampler("displacementMap", oceanFFT.getDisplacement(), oceanSampler, 0);
		//shaders["ocean-shader"]->setSampler("reflectionMap", reflection, oceanSampler, 1);
		shaders["ocean-shader"]->setSampler("reflectionMap", skybox, skyboxSampler, 1);
		shaders["ocean-shader"]->setSampler("foldingMap", oceanFFT.getFoldingMap(), oceanSampler, 2);
		shaders["ocean-shader"]->setSampler("foam", foam, oceanSampler, 3);
		//oceanShader.setSampler("dudv", oceanDUDV, oceanSampler, 4);
		context.draw(hdrTarget, *shaders["ocean-shader"], ocean.getGridArray(), primitive);

		cube.updateBuffer(1, glm::value_ptr(glm::translate(camera->getViewProjection(),
			camera->getPosition())), sizeof(glm::mat4));
		shaders["skybox-shader"]->setSampler("skybox", skybox, skyboxSampler, 0);
		context.draw(hdrTarget, *shaders["skybox-shader"], cube, GL_TRIANGLES);

		blurBuffer.update();

		context.setDrawBuffers(1);

		shaders["bloom-shader"]->setSampler("scene", hdrTexture, sampler, 0);
		shaders["bloom-shader"]->setSampler("brightBlur", brightTexture, sampler, 1);
		context.drawQuad(hdrTarget, *shaders["bloom-shader"]);
		
		shaders["tone-map-shader"]->setSampler("screen", hdrTexture, sampler, 0);
		context.drawQuad(hdrTarget, *shaders["tone-map-shader"]);
		
		shaders["screen-render-shader"]->setSampler("screen", renderWater
				? hdrTexture : oceanFFT.getFoldingMap(), sampler, 0);
		context.drawQuad(screen, *shaders["screen-render-shader"]);

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
			case GLFW_KEY_Z:
				beaufort = beaufort - 1.f >= 0.f ? beaufort - 1.f : 0.f;
				break;
			case GLFW_KEY_X:
				beaufort = beaufort + 1.f <= 12.f ? beaufort + 1.f : 12.f;
				break;
		}
	}
}

void onMouseClicked(GLFWwindow* window, int button, int action, int mods) {
	
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

	if (glfwGetKey(display.getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		dx *= 10.f;
		dy *= 10.f;
		dz *= 10.f;
	}

	camera->move(dx, dy, dz);
}

void createCube(IndexedModel& model) {
	model.allocateElement(3); // position
	model.setInstancedElementStartIndex(1);
	model.allocateElement(16); // transform

	for (float z = -1.f; z <= 1.f; z += 2.f) {
		for (float y = -1.f; y <= 1.f; y += 2.f) {
			for (float x = -1.f; x <= 1.f; x += 2.f) {
				model.addElement3f(0, x, y, z);
			}
		}
	}

	// back
	model.addIndices3i(0, 1, 2);
	model.addIndices3i(3, 2, 1);

	// front
	model.addIndices3i(6, 5, 4);
	model.addIndices3i(5, 6, 7);

	// bottom
	model.addIndices3i(4, 1, 0);
	model.addIndices3i(1, 4, 5);

	// top
	model.addIndices3i(2, 3, 6);
	model.addIndices3i(7, 6, 3);

	// left
	model.addIndices3i(0, 2, 6);
	model.addIndices3i(6, 4, 0);

	model.addIndices3i(1, 5, 7);
	model.addIndices3i(7, 3, 1);
}

void loadShaders(RenderContext& context,
		std::unordered_map<std::string, std::shared_ptr<Shader>>& shaders) {
	const std::string shaderNames[] = {"basic-shader", "ocean-shader",
			"skybox-shader", "screen-render-shader", "tone-map-shader",
			"gaussian-blur-shader", "bloom-shader"};

	std::stringstream fileData;

	for (uint32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(shaderNames); ++i) {
		std::string fileName = "./src/" + shaderNames[i] + ".glsl";

		fileData.str("");
		Util::resolveFileLinking(fileData, fileName, "#include");

		shaders.insert( std::make_pair( shaderNames[i],
				std::make_shared<Shader>(context, fileData.str()) ) );
	}
}

void setBeaufortLevel(OceanFFT& oceanFFT, UniformBuffer& oceanDataBuffer,
		const glm::vec2& windDir, float beaufortLevel) {
	float normBF = beaufortLevel / 12.f;

	float f[] = {1.f + normBF, 0.01f, 0.5f + 0.5f * normBF};
	oceanDataBuffer.update(f, 4 * sizeof(glm::vec4) + sizeof(glm::vec3), sizeof(f));

	oceanFFT.setOceanParams(2.f * (beaufortLevel + 1.f), windDir,
			10.f * (beaufortLevel + 1.f), 0.5f);
	oceanFFT.setTimeScale(5.f);
	oceanFFT.setFoldingParams(0.5f + 0.5f * normBF,
			0.2f + 0.05f * normBF, 0.0075f + 0.0025f * normBF);
}
