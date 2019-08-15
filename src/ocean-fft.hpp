#pragma once

#include "shader.hpp"

#include <GLM/glm.hpp>

class OceanFFT {
	public:
		OceanFFT(RenderContext& context, int32 N, int32 L, bool choppy);

		void init(float amplitude, const glm::vec2& direction,
				float intensity, float capillarSuppressFactor);

		void update(float delta);

		inline Texture& getH0K() { return imageH0k; }
		inline Texture& getH0MinusK() { return imageH0MinusK; }
		inline Texture& getButterflyTexture() { return butterflyTexture; }

		inline Texture& getCoeffDX() { return coeffDX; }
		inline Texture& getCoeffDY() { return coeffDY; }
		inline Texture& getCoeffDZ() { return coeffDZ; }

		inline Texture& getBufferTexture() { return bufferTexture; }

		inline Texture& getDX() { return dX; }
		inline Texture& getDY() { return dY; }
		inline Texture& getDZ() { return dZ; }

		~OceanFFT();
	private:
		RenderContext* context;

		int32 N;
		int32 L;
		int32 log2N;

		bool choppy;

		bool altBuffer;

		float timeCounter;

		Shader* hktShader;
		Shader* butterflyShader;
		Shader* inversionShader;

		Texture imageH0k;
		Texture imageH0MinusK;
		Texture butterflyTexture;

		Texture coeffDX, coeffDY, coeffDZ;
		Texture dX, dY, dZ;

		Texture bufferTexture;

		void computeIFFT(Texture&, Texture&, const glm::vec3&);
};
