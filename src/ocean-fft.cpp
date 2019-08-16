#include "ocean-fft.hpp"

#include "util.hpp"

#include <cmath>
#include <GLM/gtc/type_ptr.hpp>

static void initH0k(RenderContext& context, Texture& imageH0k, Texture& imageH0MinusK,
		int32 N, int32 L, float amplitude, const glm::vec2& direction,
		float intensity, float capillarSuppressFactor);
static void initButterflyTexture(RenderContext& context, int32 N, Texture& butterflyTexture);

OceanFFT::OceanFFT(RenderContext& context, int32 N, int32 L, bool choppy)
		: context(&context)
		, N(N)
		, L(L)
		, log2N(std::ilogb(N))
		, choppy(choppy)
		, altBuffer(false)
		, timeCounter(0.f)
		, butterflyShader(butterflyShader)
		, imageH0k(context, N, N, GL_RGBA32F)
		, imageH0MinusK(context, N, N, GL_RGBA32F)
		, butterflyTexture(context, log2N, N, GL_RGBA32F)
		, coeffDX(context, N, N, GL_RGBA32F)
		, coeffDY(context, N, N, GL_RGBA32F)
		, coeffDZ(context, N, N, GL_RGBA32F)
		, dXYZ(context, N, N, GL_RGBA32F)
		, bufferTexture(context, N, N, GL_RGBA32F) {
	std::stringstream ss;

	Util::resolveFileLinking(ss, "./src/hkt-shader.glsl", "#include");
	hktShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/butterfly-shader.glsl", "#include");
	butterflyShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/inversion-shader.glsl", "#include");
	inversionShader = new Shader(context, ss.str());
}

void OceanFFT::init(float amplitude, const glm::vec2& direction,
		float intensity, float capillarSuppressFactor) {
	initH0k(*context, imageH0k, imageH0MinusK, N, L, amplitude, direction,
			intensity, capillarSuppressFactor);
	initButterflyTexture(*context, N, butterflyTexture);

	context->awaitFinish();

	context->setShader(hktShader->getID());

	glUniform1i(hktShader->getUniform("N"), N);
	glUniform1i(hktShader->getUniform("L"), L);

	context->setShader(inversionShader->getID());
	glUniform1i(inversionShader->getUniform("N"), N);
}

void OceanFFT::update(float delta) {
	// compute hkt
	context->setShader(hktShader->getID());

	glUniform1f(hktShader->getUniform("t"), timeCounter);

	hktShader->bindComputeTexture(coeffDX, 0, GL_READ_WRITE, GL_RGBA32F);
	hktShader->bindComputeTexture(coeffDY, 1, GL_READ_WRITE, GL_RGBA32F);
	hktShader->bindComputeTexture(coeffDZ, 2, GL_READ_WRITE, GL_RGBA32F);
	
	hktShader->bindComputeTexture(imageH0k, 3, GL_READ_ONLY, GL_RGBA32F);
	hktShader->bindComputeTexture(imageH0MinusK, 4, GL_READ_ONLY, GL_RGBA32F);

	context->compute(*hktShader, N / 16, N / 16);
	context->awaitFinish();

	computeIFFT(coeffDY, dXYZ, glm::vec3(0, 1, 0));

	if (choppy) {
		computeIFFT(coeffDX, dXYZ, glm::vec3(1, 0, 0));
		computeIFFT(coeffDZ, dXYZ, glm::vec3(0, 0, 1));
	}

	timeCounter += delta;
}

OceanFFT::~OceanFFT() {
	delete hktShader;
	delete butterflyShader;
	delete inversionShader;
}

inline void OceanFFT::computeIFFT(Texture& coeff, Texture& output,
		const glm::vec3& mask) {
	altBuffer = false;

	context->setShader(butterflyShader->getID());

	butterflyShader->bindComputeTexture(butterflyTexture, 0, GL_READ_ONLY, GL_RGBA32F);
	butterflyShader->bindComputeTexture(coeff, 1, GL_READ_WRITE, GL_RGBA32F);
	butterflyShader->bindComputeTexture(bufferTexture, 2, GL_READ_WRITE, GL_RGBA32F);

	// 1D FFT horizontal
	glUniform1i(butterflyShader->getUniform("direction"), 0);

	for (uint32 i = 0; i < log2N; ++i) {
		glUniform1i(butterflyShader->getUniform("bufferNum"), altBuffer);
		glUniform1i(butterflyShader->getUniform("stage"), i);

		context->compute(*butterflyShader, N / 16, N / 16);
		context->awaitFinish();

		altBuffer = !altBuffer;
	}

	// 1D FFT vertical
	glUniform1i(butterflyShader->getUniform("direction"), 1);

	for (uint32 i = 0; i < log2N; ++i) {
		glUniform1i(butterflyShader->getUniform("bufferNum"), altBuffer);
		glUniform1i(butterflyShader->getUniform("stage"), i);

		context->compute(*butterflyShader, N / 16, N / 16);
		context->awaitFinish();

		altBuffer = !altBuffer;
	}

	context->setShader(inversionShader->getID());

	glUniform1i(inversionShader->getUniform("bufferNum"), altBuffer);
	glUniform3fv(inversionShader->getUniform("mask"), 1, glm::value_ptr(mask));

	inversionShader->bindComputeTexture(output, 0, GL_READ_WRITE, GL_RGBA32F);

	context->compute(*inversionShader, N / 16, N / 16);
	context->awaitFinish();
}

static void initH0k(RenderContext& context, Texture& imageH0k, Texture& imageH0MinusK,
		int32 N, int32 L, float amplitude, const glm::vec2& direction,
		float intensity, float capillarSuppressFactor) {
	std::stringstream ss;
	Bitmap bmp;

	// init resources
	Util::resolveFileLinking(ss, "./src/h0k-shader.glsl", "#include");
	Shader h0kShader(context, ss.str());

	bmp.load("./res/Noise256_0.jpg");
	Texture noise0(context, bmp, GL_RGBA32F);

	bmp.load("./res/Noise256_1.jpg");
	Texture noise1(context, bmp, GL_RGBA32F);

	bmp.load("./res/Noise256_2.jpg");
	Texture noise2(context, bmp, GL_RGBA32F);

	bmp.load("./res/Noise256_3.jpg");
	Texture noise3(context, bmp, GL_RGBA32F);

	Sampler noiseSampler(context);

	// bind uniforms/textures
	context.setShader(h0kShader.getID());

	h0kShader.bindComputeTexture(imageH0k, 0, GL_WRITE_ONLY, GL_RGBA32F);
	h0kShader.bindComputeTexture(imageH0MinusK, 1, GL_WRITE_ONLY, GL_RGBA32F);

	h0kShader.setSampler("noise_r0", noise0, noiseSampler, 2);
	h0kShader.setSampler("noise_i0", noise1, noiseSampler, 3);
	h0kShader.setSampler("noise_r1", noise2, noiseSampler, 4);
	h0kShader.setSampler("noise_i1", noise3, noiseSampler, 5);

	glUniform1i(h0kShader.getUniform("N"), N);
	glUniform1i(h0kShader.getUniform("L"), L);
	glUniform1f(h0kShader.getUniform("amplitude"), amplitude);
	glUniform1f(h0kShader.getUniform("intensity"), intensity);
	glUniform2fv(h0kShader.getUniform("direction"), 1, glm::value_ptr(direction));
	glUniform1f(h0kShader.getUniform("l"), capillarSuppressFactor);

	// dispatch computation
	context.compute(h0kShader, N / 16, N / 16);
}

static void initButterflyTexture(RenderContext& context, int32 N,
		Texture& butterflyTexture) {
	int32 bitReversedIndices[N];
	int32 bits = std::ilogb(N);

	for (uint32 i = 0; i < N; ++i) {
		bitReversedIndices[i] = Util::rotateLeft(Util::reverseBits(i), bits);
	}

	// init resources
	std::stringstream ss;
	Util::resolveFileLinking(ss, "./src/butterfly-texture-shader.glsl", "#include");
	Shader btShader(context, ss.str());

	ShaderStorageBuffer bitReversedBuffer(context, N * sizeof(int32),
			GL_STATIC_DRAW, bitReversedIndices);

	// bind uniforms/textures
	context.setShader(btShader.getID());

	btShader.setShaderStorageBuffer("bitReversedIndices", bitReversedBuffer, 1, 1);
	btShader.bindComputeTexture(butterflyTexture, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glUniform1i(btShader.getUniform("N"), N);

	context.compute(btShader, bits, N / 16);
}
