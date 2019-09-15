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

#include "vertex-array.hpp"
#include "shader.hpp"
#include "util.hpp"

#include "transform-feedback.hpp"
#include "input-stream-buffer.hpp"

#include "ocean.hpp"
#include "ocean-fft.hpp"
#include "ocean-projector.hpp"

#include "asset-loader.hpp"

#include "deferred-render-target.hpp"

#define MOVE_SPEED	0.5f

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

struct Particle {
	float position[3];
	float velocity[3];
	float timeToLive;
};

void testTFB(RenderContext& context) {
	// render
	/*context.setRasterizerDiscard(true);
	context.beginTransformFeedback(particleShader, tfb, GL_POINTS);

	context.drawArray(particleShader, buffers, 0, GL_POINTS, 1, 3);
	context.drawArray(particleShader, buffers, 0, GL_POINTS, 1, 3);

	context.endTransformFeedback();
	context.setRasterizerDiscard(false);

	context.awaitFinish();

	float result[6];

	glBindBuffer(GL_ARRAY_BUFFER, tfb.getBuffer(0));
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(result), result);
	DEBUG_LOG_TEMP("%.1f, %.1f, %.1f, %.1f, %.1f, %.1f", result[0], result[1], result[2],
			result[3], result[4], result[5]);

	for (uint32 i = 0; i < 3; ++i) {
		tfb.flip();

		context.setRasterizerDiscard(true);
		context.beginTransformFeedback(particleShader, tfb, GL_POINTS);

		//context.drawTransformFeedback(particleShader, tfb, GL_POINTS);
		glUseProgram(particleShader.getID());
		glDrawTransformFeedback(GL_POINTS, tfb.getReadFeedback());

		context.endTransformFeedback();
		context.setRasterizerDiscard(false);

		context.awaitFinish();
	}*/
}

int main() {
	Display display("MoIsT - Ocean Rendering", 1200, 900);

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

	testTFB(context);

	loadShaders(context, shaders);
	
	Ocean ocean(context, 0.f, 4.f, 256);
	OceanProjector projector(ocean, userCamera);

	UniformBuffer sceneDataBuffer(context, sizeof(glm::vec3) + sizeof(glm::vec2)
			+ sizeof(glm::mat4), GL_STREAM_DRAW, 0);
	UniformBuffer oceanDataBuffer(context, 4 * sizeof(glm::vec4)
			+ 3 * sizeof(float), GL_STREAM_DRAW, 1);
	UniformBuffer lightDataBuffer(context, 1 * sizeof(glm::vec3)
			+ 3 * sizeof(float) + sizeof(glm::vec3) + 2 * sizeof(float), GL_DYNAMIC_DRAW, 2);

	{
		float lightData[] = {0.f, 15.f, 128.f};
		lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(1, -1, 1))), sizeof(glm::vec3));
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
	context.awaitFinish();

	Bitmap bmp;
	bmp.load("./res/foam.jpg");
	Texture foam(context, bmp, GL_RGBA);

	IndexedModel cubeModel;
	createCube(cubeModel);

	VertexArray cube(context, cubeModel, GL_STATIC_DRAW);

	std::vector<IndexedModel> loadedModels;
	AssetLoader::loadAssets("./res/cube.obj", loadedModels);

	VertexArray loadedModel(context, loadedModels[0], GL_STATIC_DRAW);

	glm::mat4 blockPos(1.f);

	DDSTexture ddsTexture;
	
	//ddsTexture.load("./res/wood-planks.dds");
	ddsTexture.load("./res/bricks.dds");
	Texture diffuseMap(context, ddsTexture);

	//ddsTexture.load("./res/wood-planks-normal2.dds");
	ddsTexture.load("./res/bricks-normal.dds");
	Texture normalMap(context, ddsTexture);

	//ddsTexture.load("./res/wood-planks-roughness.dds");
	ddsTexture.load("./res/bricks-roughness.dds");
	Texture roughnessMap(context, ddsTexture);

	//ddsTexture.load("./res/wood-planks-ao.dds");
	ddsTexture.load("./res/bricks-ao.dds");
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

	const char* varyings[] = {"position1", "velocity1", "ttl1"};
	std::stringstream ss;
	Util::resolveFileLinking(ss, "./src/particle-update-shader.glsl", "#include");
	Shader particleShader(context, ss.str(), varyings,
			ARRAY_SIZE_IN_ELEMENTS(varyings), GL_INTERLEAVED_ATTRIBS);

	const Particle data[1] = {{0.f, 0.f, 0.f, 0.f, 10.f, 0.f, 1.f}};
	const uint32 elementSizes[] = {3, 3, 1};

	TransformFeedback tfb(context, ARRAY_SIZE_IN_ELEMENTS(elementSizes),
			elementSizes, 100);
	InputStreamBuffer isb(context, ARRAY_SIZE_IN_ELEMENTS(elementSizes),
			elementSizes, 10);

	VertexArray tfbCube0(context, cubeModel, tfb, 0, GL_STATIC_DRAW);
	VertexArray tfbCube1(context, cubeModel, tfbCube0, tfb, 1);

	context.setRasterizerDiscard(true);
	context.beginTransformFeedback(particleShader, tfb, GL_POINTS);

	context.drawArray(particleShader, isb, 1, GL_POINTS);

	context.endTransformFeedback();
	context.setRasterizerDiscard(false);

	context.awaitFinish();

	tfb.swapBuffers();

	uint32 numParticles = 0;
	float particleCounter = 0.f;

	while (!display.isCloseRequested()) {
		updateCameraMovement(display);
		camera->update();

		if (lockCamera) {
			projector.update();
		}
		
		sceneDataBuffer.update(glm::value_ptr(camera->getPosition()), sizeof(glm::vec3));
		sceneDataBuffer.update(glm::value_ptr(camera->getInverseVP()),
				sizeof(glm::vec3) + sizeof(glm::vec2) + 3 * sizeof(float), sizeof(glm::mat4));

		//lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(std::cos(glfwGetTime()),
		//		1, std::sin(glfwGetTime())))), sizeof(glm::vec3));
		
		if (beaufort != lastBeaufort) {
			setBeaufortLevel(oceanFFT, oceanDataBuffer, glm::vec2(1, 1), beaufort);
		}

		lastBeaufort = beaufort;

		//lightDataBuffer.update(glm::value_ptr(glm::normalize(glm::vec3(std::cos(0.2 * glfwGetTime()),
		//		std::sin(0.2 * glfwGetTime()), 0.f))),
		//		sizeof(glm::vec3));
		
		oceanFFT.addFloatingTransform(blockPos, glm::vec2(1.f, 1.f));

		oceanFFT.update(1.f / 60.f);

		oceanDataBuffer.update(projector.getCorners(), 4 * sizeof(glm::vec4));

		ocean.getGridArray().updateBuffer(1, glm::value_ptr(camera->getViewProjection()),
				sizeof(glm::mat4));

		blockPos = glm::scale(oceanFFT.getFloatingTransforms()[0],
				glm::vec3(2.f, 0.05f + 0.9f * (beaufort / 12.f), 2.f));
		blockPos[3][1] -= 0.45f * (beaufort / 12.f);
		glm::mat4 mats[] = {camera->getViewProjection() * blockPos, blockPos};
		loadedModel.updateBuffer(4, mats, sizeof(mats));

		// BEGIN PARTICLE UPDATE
		particleCounter += 1.f / 60.f;

		if (particleCounter > 0.1f) {
			numParticles = 1;
			isb.update(data, sizeof(data));
			particleCounter = 0.f;
		}

		context.setRasterizerDiscard(true);
		context.beginTransformFeedback(particleShader, tfb, GL_POINTS);

		if (numParticles > 0) {
			context.drawArray(particleShader, isb, numParticles, GL_POINTS);
			numParticles = 0;
		}

		context.drawTransformFeedback(particleShader, tfb, GL_POINTS);

		context.endTransformFeedback();
		context.setRasterizerDiscard(false);

		context.awaitFinish();
		tfb.swapBuffers();
		// END PARTICLE UPDATE

		// BEGIN DRAW
		gBuffer.clear();

		shaders["static-mesh-deferred"]->setSampler("diffuse", diffuseMap, mipmapSampler, 0);
		shaders["static-mesh-deferred"]->setSampler("normalMap", normalMap, mipmapSampler, 1);
		shaders["static-mesh-deferred"]->setSampler("roughnessMap", roughnessMap, mipmapSampler, 2);
		shaders["static-mesh-deferred"]->setSampler("aoMap", aoMap, mipmapSampler, 3);
		//context.draw(gBuffer.getTarget(), *shaders["static-mesh-deferred"], loadedModel, GL_TRIANGLES);

		shaders["ocean-deferred"]->setSampler("displacementMap", oceanFFT.getDisplacement(), oceanSampler, 0);
		shaders["ocean-deferred"]->setSampler("foldingMap", oceanFFT.getFoldingMap(), oceanSampler, 1);
		shaders["ocean-deferred"]->setSampler("foam", foam, oceanSampler, 2);
		context.draw(gBuffer.getTarget(), *shaders["ocean-deferred"], ocean.getGridArray(), primitive);

		context.setWriteDepth(false);

		/*shaders["decal-shader"]->setMatrix4f("invMVP", glm::inverse(mats[0]));

		shaders["decal-shader"]->setSampler("depthBuffer", gBuffer.getDepthBuffer(), sampler, 0);
		shaders["decal-shader"]->setSampler("normalBuffer", gBuffer.getNormalBuffer(), sampler, 1);

		shaders["decal-shader"]->setSampler("diffuse", diffuseMap, mipmapSampler, 2);
		shaders["decal-shader"]->setSampler("normalMap", normalMap, mipmapSampler, 3);
		shaders["decal-shader"]->setSampler("roughnessMap", roughnessMap, mipmapSampler, 4);
		shaders["decal-shader"]->setSampler("aoMap", aoMap, mipmapSampler, 5);
		context.draw(gBuffer.getTarget(), *shaders["decal-shader"], loadedModel, GL_TRIANGLES);*/

		shaders["wake-shader"]->setMatrix4f("invMVP", glm::inverse(mats[0]));

		shaders["wake-shader"]->setSampler("depthBuffer", gBuffer.getDepthBuffer(), sampler, 0);
		shaders["wake-shader"]->setSampler("colorBuffer", gBuffer.getColorBuffer(), sampler, 1);
		shaders["wake-shader"]->setSampler("normalBuffer", gBuffer.getNormalBuffer(), sampler, 2);
		shaders["wake-shader"]->setSampler("lightingBuffer", gBuffer.getLightingBuffer(), sampler, 3);

		shaders["wake-shader"]->setSampler("diffuse", wake, skyboxSampler, 4);
		context.draw(gBuffer.getTarget(), *shaders["wake-shader"], loadedModel, GL_TRIANGLES);

		gBuffer.applyLighting();

		cube.updateBuffer(1, glm::value_ptr(glm::translate(camera->getViewProjection(),
			camera->getPosition())), sizeof(glm::mat4));
		shaders["skybox-deferred"]->setSampler("skybox", specularIBL, mipmapSampler, 0);
		context.draw(gBuffer.getTarget(), *shaders["skybox-deferred"], cube, GL_TRIANGLES);

		context.setBlending(RenderContext::BLEND_FUNC_SRC_ALPHA,
				RenderContext::BLEND_FUNC_DST_ALPHA);
		shaders["billboard-shader"]->setSampler("billboard", smoke, skyboxSampler, 0);
		context.drawTransformFeedback(gBuffer.getTarget(), *shaders["billboard-shader"], tfb, GL_POINTS);
		context.setBlending(RenderContext::BLEND_FUNC_NONE, RenderContext::BLEND_FUNC_NONE);

		gBuffer.flush();

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
		dx *= 0.1f;
		dy *= 0.1f;
		dz *= 0.1f;
	}

	camera->move(dx, dy, dz);
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
