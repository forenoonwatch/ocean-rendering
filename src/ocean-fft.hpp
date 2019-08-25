#pragma once

#include "shader.hpp"

#include <GLM/glm.hpp>

class OceanFFTSeed {
	public:
		OceanFFTSeed(RenderContext& context, int32 N, int32 L);

		void setParams(float amplitude, const glm::vec2& direction,
				float intensity, float capillarSuppressFactor);

		inline Texture& getH0K() { return imageH0k; }

		~OceanFFTSeed();
	private:
		NULL_COPY_AND_ASSIGN(OceanFFTSeed);

		RenderContext* context;

		uint32 computeSpace;

		Shader* h0kShader;

		Texture* noise[4];

		Texture imageH0k;

		Sampler noiseSampler;
};

class OceanFFT {
	public:
		OceanFFT(RenderContext& context, int32 N, int32 L,
				bool choppy, float timeScale);

		void setOceanParams(float amplitude, const glm::vec2& direction,
				float intensity, float capillarSuppressFactor);

		void update(float delta);

		inline Texture& getH0K() { return fftSeed.getH0K(); }
		inline Texture& getButterflyTexture() { return butterflyTexture; }

		inline Texture& getCoeffDX() { return coeffDX; }
		inline Texture& getCoeffDY() { return coeffDY; }
		inline Texture& getCoeffDZ() { return coeffDZ; }

		inline Texture& getBufferTexture() { return bufferTexture; }

		inline Texture& getDisplacement() { return displacement; }

		inline Texture& getFoldingMap() { return foldingMap; }

		~OceanFFT();
	private:
		NULL_COPY_AND_ASSIGN(OceanFFT);

		RenderContext* context;

		int32 N;
		int32 L;
		int32 log2N;

		bool choppy;
		float timeScale;

		bool altBuffer;
		float timeCounter;

		Shader* hktShader;
		Shader* butterflyShader;
		Shader* inversionShader;
		Shader* foldingShader;

		OceanFFTSeed fftSeed;

		Texture butterflyTexture;

		Texture coeffDX, coeffDY, coeffDZ;
		Texture displacement;

		Texture bufferTexture;

		Texture foldingMap;

		void computeIFFT(Texture&, Texture&, const glm::vec3&);
};
