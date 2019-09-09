#include "deferred-render-target.hpp"

#include "util.hpp"

DeferredRenderTarget::DeferredRenderTarget(RenderContext& context,
			uint32 width, uint32 height, CubeMap& skybox,
			CubeMap& diffuseIBL, CubeMap& specularIBL,
			Texture& brdfLUT)
		: context(&context)
		, colorBuffer(context, width, height, GL_RGBA32F)
		, normLightBuffer(context, width, height, GL_RGBA32F)
		, brightBuffer(context, width, height, GL_RGBA32F)
		, depthBuffer(context, width, height, GL_DEPTH_COMPONENT,
				nullptr, GL_DEPTH_COMPONENT, GL_FLOAT)
		, target(context, colorBuffer, GL_COLOR_ATTACHMENT0)
		, screen(context, width, height)
		, sampler(context, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, skyboxSampler(context, GL_LINEAR, GL_LINEAR)
		, mipmapSampler(context, GL_LINEAR_MIPMAP_LINEAR,
				GL_LINEAR_MIPMAP_LINEAR)
		, skybox(&skybox)
		, diffuseIBL(&diffuseIBL)
		, specularIBL(&specularIBL)
		, brdfLUT(&brdfLUT) {
	target.addTextureTarget(normLightBuffer, GL_COLOR_ATTACHMENT0, 1);
	target.addTextureTarget(brightBuffer, GL_COLOR_ATTACHMENT0, 2);

	target.addTextureTarget(depthBuffer, GL_DEPTH_ATTACHMENT);

	std::stringstream ss;

	ss.str("");
	Util::resolveFileLinking(ss, "./src/screen-render-shader.glsl", "#include");
	screenRenderShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/bloom-shader.glsl", "#include");
	bloomShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/gaussian-blur-shader.glsl", "#include");
	blurShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/tone-map-shader.glsl", "#include");
	toneMapShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/deferred-lighting.glsl", "#include");
	lightingShader = new Shader(context, ss.str());

	bloomBlur = new GaussianBlur(context, *blurShader, brightBuffer);
}

void DeferredRenderTarget::clear() {
	screen.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	target.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setDrawBuffers(3);
}

void DeferredRenderTarget::applyLighting() {
	context->setWriteDepth(false);

	// apply lighting
	lightingShader->setSampler("colorBuffer", colorBuffer, sampler, 0);
	lightingShader->setSampler("normLightBuffer", normLightBuffer, sampler, 1);
	lightingShader->setSampler("depthBuffer", depthBuffer, sampler, 2);
	//lightingShader->setSampler("reflectionMap", *skybox, skyboxSampler, 3);
	
	lightingShader->setSampler("irradianceMap", *diffuseIBL, mipmapSampler, 3);
	lightingShader->setSampler("prefilterMap", *specularIBL, skyboxSampler, 4);
	lightingShader->setSampler("brdfLUT", *brdfLUT, sampler, 5);

	context->drawQuad(target, *lightingShader);
}

void DeferredRenderTarget::flush() {			
	// calculate bloom blur
	bloomBlur->update();

	context->setDrawBuffers(1);

	// draw bloom
	bloomShader->setSampler("scene", colorBuffer, sampler, 0);
	bloomShader->setSampler("brightBlur", brightBuffer, sampler, 1);
	context->drawQuad(target, *bloomShader);
	
	// tone map colors
	toneMapShader->setSampler("screen", colorBuffer, sampler, 0);
	context->drawQuad(target, *toneMapShader);

	// draw to screen
	screenRenderShader->setSampler("screen", colorBuffer, sampler, 0);
	context->drawQuad(screen, *screenRenderShader);

	context->setWriteDepth(true);
}

DeferredRenderTarget::~DeferredRenderTarget() {
	delete screenRenderShader;
	delete bloomShader;
	delete blurShader;
	delete toneMapShader;
	delete lightingShader;

	delete bloomBlur;
}
