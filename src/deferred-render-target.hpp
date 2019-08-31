#pragma once

#include "render-target.hpp"
#include "gaussian-blur.hpp"
#include "vertex-array.hpp"

class DeferredRenderTarget {
	public:
		DeferredRenderTarget(RenderContext& context,
				uint32 width, uint32 height, CubeMap& skybox);

		void clear();
		void applyLighting();
		void flush();

		inline void setSkybox(CubeMap& skybox) { this->skybox = &skybox; }

		inline RenderTarget& getTarget() { return target; }

		~DeferredRenderTarget();
	private:
		NULL_COPY_AND_ASSIGN(DeferredRenderTarget);

		RenderContext* context;

		Texture colorBuffer;
		Texture normLightBuffer;
		Texture brightBuffer;
		Texture depthBuffer;

		RenderTarget target;
		RenderTarget screen;
		
		Sampler sampler;
		Sampler skyboxSampler;
		
		Shader* screenRenderShader;
		Shader* bloomShader;
		Shader* blurShader;
		Shader* toneMapShader;
		Shader* lightingShader;

		GaussianBlur* bloomBlur;

		CubeMap* skybox;
};
