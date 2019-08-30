#pragma once

#include "render-target.hpp"
#include "gaussian-blur.hpp"

class DeferredRenderTarget {
	public:
		DeferredRenderTarget(RenderContext& context,
				uint32 width, uint32 height);

		void clear();
		void flush();

		inline RenderTarget& getScreen() { return screen; }
		inline RenderTarget& getTarget() { return target; }

		~DeferredRenderTarget();
	//private:
		NULL_COPY_AND_ASSIGN(DeferredRenderTarget);

		RenderContext* context;

		Texture colorBuffer;
		Texture positionBuffer;
		Texture normalBuffer;
		Texture brightBuffer;
		Texture depthBuffer;

		RenderTarget target;
		RenderTarget screen;
		
		Sampler sampler;

		Shader* screenRenderShader;
		Shader* bloomShader;
		Shader* blurShader;
		Shader* toneMapShader;

		GaussianBlur* bloomBlur;
};
