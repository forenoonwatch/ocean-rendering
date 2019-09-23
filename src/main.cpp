#include <cmath>
#include <cstdio>
#include <algorithm>

#include <memory>
#include <unordered_map>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "application.hpp"

#include "camera.hpp"

#include "vertex-array.hpp"
#include "shader.hpp"
#include "util.hpp"

#include "ocean.hpp"
#include "ocean-fft.hpp"
#include "ocean-projector.hpp"

#include "asset-loader.hpp"

#include "deferred-render-target.hpp"

#include "particle-system.hpp"
#include "wake-system.hpp"

#define MOVE_SPEED	0.5f

void updateCameraMovement();
void updateInput();
void cameraFollow(const glm::vec3&, float);

void createCube(IndexedModel&);
void loadShaders(RenderContext&,
		std::unordered_map<std::string, std::shared_ptr<Shader>>&);

void setBeaufortLevel(OceanFFT&, UniformBuffer&, const glm::vec2&, float);

Camera* camera;
bool lockCamera;
bool renderWater;
uint32 primitive;

float beaufort;

Window* displayPtr;

int main() {
	Application::init();
	Window display("MoIsT - Ocean Rendering", 1200, 900);
	displayPtr = &display;

	lockCamera = true;
	renderWater = true;
	primitive = GL_TRIANGLES;

	beaufort = 0.f;
	float lastBeaufort = 0.f;

	float fieldOfView = glm::radians(70.f);
	float aspectRatio = (float)display.getWidth() / (float)display.getHeight();
	float zNear = 0.1f;
	float zFar = 1000.f;

	Camera userCamera(fieldOfView, aspectRatio, zNear, 10.f * zFar);
	camera = &userCamera;

	RenderContext context;
	std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

	loadShaders(context, shaders);
	
	Ocean ocean(context, 0.f, 4.f, 256);
	OceanProjector projector(ocean, userCamera);

	UniformBuffer sceneDataBuffer(context, sizeof(glm::vec3) + sizeof(glm::vec2)
			+ 2 * sizeof(glm::mat4), GL_STREAM_DRAW, 0);
	UniformBuffer oceanDataBuffer(context, 4 * sizeof(glm::vec4)
			+ 3 * sizeof(float), GL_STREAM_DRAW, 1);
	UniformBuffer lightDataBuffer(context, 1 * sizeof(glm::vec3)
			+ 3 * sizeof(float) + sizeof(glm::vec3) + 2 * sizeof(float), GL_DYNAMIC_DRAW, 2);

	{
		float lightData[] = {0.f, 15.f, 128.f};
		lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(1, -1, 1))), sizeof(glm::vec3));
		lightDataBuffer.update(lightData, sizeof(glm::vec3), sizeof(lightData));

		float fogData[] = {202.f / 255.f, 243.f / 255.f, 246.f / 255.f,
			0.001f, 2.f};

		lightDataBuffer.update(fogData, sizeof(glm::vec3) + sizeof(lightData)
				+ 2 * sizeof(float), sizeof(fogData));
	}

	Sampler oceanSampler(context, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	Sampler sampler(context, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
	Sampler skyboxSampler(context, GL_LINEAR, GL_LINEAR);

	OceanFFT oceanFFT(context, 256, 1000, true, 5.f);
	context.awaitFinish();

	Bitmap bmp;
	bmp.load("./res/foam.jpg");
	Texture foam(context, bmp, GL_RGBA);

	IndexedModel cubeModel;
	createCube(cubeModel);

	VertexArray cube(context, cubeModel, GL_STATIC_DRAW);

	std::vector<IndexedModel> loadedModels;
	//AssetLoader::loadAssets("./res/cube.obj", loadedModels);
	AssetLoader::loadAssets("./res/hull.fbx", loadedModels);

	VertexArray loadedModel(context, loadedModels[0], GL_STATIC_DRAW);

	glm::mat4 blockPos(1.f);

	DDSTexture ddsTexture;
	
	ddsTexture.load("./res/wood-planks.dds");
	//ddsTexture.load("./res/bricks.dds");
	Texture diffuseMap(context, ddsTexture);

	ddsTexture.load("./res/wood-planks-normal2.dds");
	//ddsTexture.load("./res/bricks-normal.dds");
	Texture normalMap(context, ddsTexture);

	ddsTexture.load("./res/wood-planks-roughness.dds");
	//ddsTexture.load("./res/bricks-roughness.dds");
	Texture roughnessMap(context, ddsTexture);

	ddsTexture.load("./res/wood-planks-ao.dds");
	//ddsTexture.load("./res/bricks-ao.dds");
	Texture aoMap(context, ddsTexture);

	//ddsTexture.load("./res/sargasso-diffuse.dds");
	ddsTexture.load("./res/skybox-diffuse.dds");
	CubeMap diffuseIBL(context, ddsTexture);

	//ddsTexture.load("./res/sargasso-specular.dds");
	ddsTexture.load("./res/skybox-specular.dds");
	CubeMap specularIBL(context, ddsTexture);


	bmp.load("./res/schlick-brdf.png");
	Texture brdfLUT(context, bmp, GL_RGBA);

	bmp.load("./res/wake.png");
	Texture wake(context, bmp, GL_RGBA);

	bmp.load("./res/smoke.png");
	Texture smoke(context, bmp, GL_RGBA);

	DeferredRenderTarget gBuffer(context, display.getWidth(), display.getHeight(),
			diffuseIBL, specularIBL, brdfLUT);

	sceneDataBuffer.update(glm::value_ptr(glm::vec2(display.getWidth(), display.getHeight())),
			sizeof(glm::vec3) + sizeof(float), sizeof(glm::vec2));

	Sampler mipmapSampler(context, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	
	ParticleSystem particleSystem(context, 100, 10);
	WakeSystem wakeSystem(context, oceanFFT.getDisplacement(), oceanSampler, 300, 10);

	//VertexArray tfbCube0(context, cubeModel, tfb, 0, GL_STATIC_DRAW);
	//VertexArray tfbCube1(context, cubeModel, tfbCube0, tfb, 1);

	float wakeCounter = 0.f;
	float particleCounter = 0.f;

	const glm::vec4 bowOffset(0.f, 0.f, -8.f, 1.f);
	glm::vec4 lastBowPos = bowOffset;

	while (!display.isCloseRequested()) {
		glm::vec3 shipPos = glm::vec3(blockPos[3]) / blockPos[3][3];

		updateCameraMovement();
		updateInput();
		//cameraFollow(shipPos, 10.f);
		camera->update();

		if (lockCamera) {
			projector.update();
		}
		
		sceneDataBuffer.update(glm::value_ptr(camera->getPosition()), sizeof(glm::vec3));
		sceneDataBuffer.update(glm::value_ptr(camera->getViewProjection()),
				sizeof(glm::vec3) + sizeof(glm::vec2) + 3 * sizeof(float), sizeof(glm::mat4));
		sceneDataBuffer.update(glm::value_ptr(camera->getInverseVP()),
				sizeof(glm::vec3) + sizeof(glm::vec2) + 3 * sizeof(float)
				+ sizeof(glm::mat4), sizeof(glm::mat4));

		//lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(std::cos(glfwGetTime()),
		//		1, std::sin(glfwGetTime())))), sizeof(glm::vec3));
		
		if (beaufort != lastBeaufort) {
			setBeaufortLevel(oceanFFT, oceanDataBuffer, glm::vec2(1, 1), beaufort);
		}

		lastBeaufort = beaufort;

		//lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(std::cos(0.2 * glfwGetTime()),
		//		std::sin(0.2 * glfwGetTime()), 0.f))),
		//		sizeof(glm::vec3));
		
		oceanFFT.addFloatingTransform(blockPos, glm::vec2(2.5f, 5.f));

		oceanFFT.update(1.f / 60.f);

		oceanDataBuffer.update(projector.getCorners(), 4 * sizeof(glm::vec4));

		ocean.getGridArray().updateBuffer(1, glm::value_ptr(camera->getViewProjection()),
				sizeof(glm::mat4));

		//blockPos = glm::scale(oceanFFT.getFloatingTransforms()[0],
		//		glm::vec3(2.f, 0.05f + 0.9f * (beaufort / 12.f), 2.f));
		//blockPos[3][1] -= 0.45f * (beaufort / 12.f);
		blockPos = oceanFFT.getFloatingTransforms()[0];
		blockPos *= glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -0.1f)),
				0.001f, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 mats[] = {camera->getViewProjection() * blockPos, blockPos};
		loadedModel.updateBuffer(4, mats, sizeof(mats));

		// BEGIN PARTICLE UPDATE
		wakeCounter += 1.f / 60.f;
		particleCounter += 1.f / 60.f;

		if (wakeCounter > 0.08f) {
			//particleSystem.drawParticle(particle);
			
			const glm::vec4 transScale(1.f, 0.f, 1.f, 3.5f);
			glm::vec2 shipRight(blockPos[0][0], blockPos[0][2]);
			
			wakeSystem.drawWake(-shipRight, transScale, glm::translate(blockPos,
						glm::vec3(-1.5f, 0.f, -6.5f)), 5.f);

			wakeSystem.drawWake(shipRight, transScale, glm::translate(blockPos,
						glm::vec3(1.5f, 0.f, -6.5f)), 5.f);

			wakeCounter = 0.f;
		}

		glm::vec4 bowPos = blockPos * bowOffset;
		float bowImpulse = bowPos.y - lastBowPos.y;
		lastBowPos = bowPos;

		if (bowImpulse > 0.01f && particleCounter >= 0.04f) {
			particleCounter = 0.f;

			const glm::vec4 transScale(1.f, 0.f, 0.5f, 1.2f);

			glm::vec3 velL = -7.f * glm::vec3(blockPos[0]) + glm::vec3(0.f, 10.f, 0.f);
			glm::vec3 velR = 7.f * glm::vec3(blockPos[0]) + glm::vec3(0.f, 10.f, 0.f);

			particleSystem.drawParticle(glm::vec3(blockPos * glm::vec4(-0.3f, -0.5f, -7.5f, 1.f)),
					velL, glm::vec3(0.f, -50.f, 0.f), transScale, 0.5f);

			particleSystem.drawParticle(glm::vec3(blockPos * glm::vec4(0.3f, -0.5f, -7.5f, 1.f)),
					velR, glm::vec3(0.f, -50.f, 0.f), transScale, 0.5f);
		}

		particleSystem.update();
		wakeSystem.update();
		// END PARTICLE UPDATE

		// BEGIN DRAW
		gBuffer.clear();

		shaders["ocean-deferred"]->setSampler("displacementMap", oceanFFT.getDisplacement(), oceanSampler, 0);
		shaders["ocean-deferred"]->setSampler("foldingMap", oceanFFT.getFoldingMap(), oceanSampler, 1);
		shaders["ocean-deferred"]->setSampler("foam", foam, oceanSampler, 2);
		context.draw(gBuffer.getTarget(), *shaders["ocean-deferred"], ocean.getGridArray(), primitive);

		wakeSystem.draw(gBuffer, wake, skyboxSampler);

		shaders["static-mesh-deferred"]->setSampler("diffuse", diffuseMap, mipmapSampler, 0);
		shaders["static-mesh-deferred"]->setSampler("normalMap", normalMap, mipmapSampler, 1);
		shaders["static-mesh-deferred"]->setSampler("roughnessMap", roughnessMap, mipmapSampler, 2);
		shaders["static-mesh-deferred"]->setSampler("aoMap", aoMap, mipmapSampler, 3);
		context.draw(gBuffer.getTarget(), *shaders["static-mesh-deferred"], loadedModel, GL_TRIANGLES);

		/*shaders["decal-shader"]->setMatrix4f("invMVP", glm::inverse(mats[0]));

		shaders["decal-shader"]->setSampler("depthBuffer", gBuffer.getDepthBuffer(), sampler, 0);
		shaders["decal-shader"]->setSampler("normalBuffer", gBuffer.getNormalBuffer(), sampler, 1);

		shaders["decal-shader"]->setSampler("diffuse", diffuseMap, mipmapSampler, 2);
		shaders["decal-shader"]->setSampler("normalMap", normalMap, mipmapSampler, 3);
		shaders["decal-shader"]->setSampler("roughnessMap", roughnessMap, mipmapSampler, 4);
		shaders["decal-shader"]->setSampler("aoMap", aoMap, mipmapSampler, 5);
		context.draw(gBuffer.getTarget(), *shaders["decal-shader"], loadedModel, GL_TRIANGLES);*/

		/*shaders["wake-shader"]->setMatrix4f("invMVP", glm::inverse(mats[0]));
		//shaders["wake-shader"]->setMatrix4f("invMVP", glm::inverse(blockPos) * camera->getInverseVP());

		shaders["wake-shader"]->setSampler("depthBuffer", gBuffer.getDepthBuffer(), sampler, 0);
		shaders["wake-shader"]->setSampler("colorBuffer", gBuffer.getColorBuffer(), sampler, 1);
		shaders["wake-shader"]->setSampler("normalBuffer", gBuffer.getNormalBuffer(), sampler, 2);
		shaders["wake-shader"]->setSampler("lightingBuffer", gBuffer.getLightingBuffer(), sampler, 3);

		shaders["wake-shader"]->setSampler("diffuse", wake, skyboxSampler, 4);
		context.draw(gBuffer.getTarget(), *shaders["wake-shader"], loadedModel, GL_TRIANGLES);*/

		gBuffer.applyLighting();

		cube.updateBuffer(1, glm::value_ptr(glm::translate(camera->getViewProjection(),
			camera->getPosition())), sizeof(glm::mat4));
		shaders["skybox-deferred"]->setSampler("skybox", specularIBL, mipmapSampler, 0);
		context.draw(gBuffer.getTarget(), *shaders["skybox-deferred"], cube, GL_TRIANGLES);
		
		particleSystem.draw(gBuffer.getTarget(), smoke, skyboxSampler);

		gBuffer.flush();

		display.render();
		Application::pollEvents();
	}

	Application::destroy();

	return 0;
}

void updateCameraMovement() {
	static double lastX = 0.0;
	static double lastY = 0.0;

	float dx = 0.f, dy = 0.f, dz = 0.f;

	if (Application::isKeyDown(Input::KEY_W)) {
		dz -= MOVE_SPEED;
	}
	
	if (Application::isKeyDown(Input::KEY_S)) {
		dz += MOVE_SPEED;
	}

	if (Application::isKeyDown(Input::KEY_A)) {
		dx -= MOVE_SPEED;
	}

	if (Application::isKeyDown(Input::KEY_D)) {
		dx += MOVE_SPEED;
	}

	if (Application::isKeyDown(Input::KEY_Q)) {
		dy -= MOVE_SPEED;
	}
	
	if (Application::isKeyDown(Input::KEY_E)) {
		dy += MOVE_SPEED;
	}

	if (Application::isKeyDown(Input::KEY_LEFT_SHIFT)) {
		dx *= 0.1f;
		dy *= 0.1f;
		dz *= 0.1f;
	}

	camera->move(dx, dy, dz);

	if (Application::isMouseDown(Input::MOUSE_BUTTON_RIGHT)) {
		camera->rotate((float)( (lastY - Application::getMouseY()) * 0.01 ),
				(float)( (lastX - Application::getMouseX()) * 0.01 ));
	}

	lastX = Application::getMouseX();
	lastY = Application::getMouseY();
}

void updateInput() {
	if (Application::getKeyPressed(Input::KEY_R)) {
		lockCamera = !lockCamera;
	}

	if (Application::getKeyPressed(Input::KEY_Z)) {
		beaufort = beaufort - 1.f >= 0.f ? beaufort - 1.f : 0.f;
	}

	if (Application::getKeyPressed(Input::KEY_X)) {
		beaufort = beaufort + 1.f <= 12.f ? beaufort + 1.f : 12.f;
	}
}

void cameraFollow(const glm::vec3& target, float distance) {
	//glm::vec3 diff = glm::normalize(target - camera->getPosition());
	//diff *= distance;
	glm::vec3 diff = target + glm::vec3(0, distance, -distance);

	camera->setPosition(diff);
}

void createCube(IndexedModel& model) {
	model.allocateElement(3); // position
	model.allocateElement(16); // transform
	
	model.setInstancedElementStartIndex(1);

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

	// right
	model.addIndices3i(1, 5, 7);
	model.addIndices3i(7, 3, 1);
}

void loadShaders(RenderContext& context,
		std::unordered_map<std::string, std::shared_ptr<Shader>>& shaders) {
	const std::string shaderNames[] = {"static-mesh-deferred",
			"ocean-deferred", "skybox-deferred", "billboard-shader",
			"ocean-surface-transform", "decal-shader", "wake-shader"};

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

	float f[] = {1.f + normBF, 0.01f, 1.f + 0.5f * normBF};
	oceanDataBuffer.update(f, 4 * sizeof(glm::vec4), sizeof(f));

	oceanFFT.setOceanParams(2.f * (beaufortLevel + 1.f), windDir,
			10.f * (beaufortLevel + 1.f), 0.5f);
	oceanFFT.setTimeScale(6.f);
	//oceanFFT.setTimeScale(0.f);
	oceanFFT.setFoldingParams(0.5f + 0.5f * normBF,
			0.2f + 0.05f * normBF, 0.0075f + 0.0025f * normBF);
}
