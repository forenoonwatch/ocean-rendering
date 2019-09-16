#pragma once

#include "render-target.hpp"
#include "gaussian-blur.hpp"
#include "vertex-array.hpp"

class DeferredRenderTarget {
	public:
		DeferredRenderTarget(RenderContext& context,
				uint32 width, uint32 height,CubeMap& diffuseIBL,
				CubeMap& specularIBL, Texture& brdfLUT);

		void clear();
		void applyLighting();
		void flush();

		inline RenderTarget& getTarget() { return target; }

		inline Texture& getColorBuffer() { return colorBuffer; }
		inline Texture& getNormalBuffer() { return normalBuffer; }
		inline Texture& getLightingBuffer() { return lightingBuffer; }
		inline Texture& getDepthBuffer() { return depthBuffer; }

		inline Sampler& getSampler() { return sampler; }

		~DeferredRenderTarget();
	private:
		NULL_COPY_AND_ASSIGN(DeferredRenderTarget);

		RenderContext* context;

		Texture colorBuffer;
		Texture normalBuffer;
		Texture lightingBuffer;
		Texture brightBuffer;
		Texture depthBuffer;

		RenderTarget target;
		RenderTarget screen;
		
		Sampler sampler;
		Sampler skyboxSampler;
		Sampler mipmapSampler;
		
		Shader* screenRenderShader;
		Shader* bloomShader;
		Shader* blurShader;
		Shader* toneMapShader;
		Shader* lightingShader;

		GaussianBlur* bloomBlur;

		CubeMap* diffuseIBL;
		CubeMap* specularIBL;

		Texture* brdfLUT;
};
