#include "ocean-fft.hpp"

#include <engine/core/util.hpp>

#include <cmath>
#include <cstring>

#define MAX_FLOATING_OBJECTS 128
#define FLOATING_BUF_SIZE (MAX_FLOATING_OBJECTS * sizeof(glm::mat4) \
		+ MAX_FLOATING_OBJECTS * sizeof(glm::vec2))

static void initButterflyTexture(RenderContext& context, int32 N, Texture& butterflyTexture);

OceanFFTSeed::OceanFFTSeed(RenderContext& context, int32 N, int32 L)
		: context(&context)
		, computeSpace(N / 16)
		, imageH0k(context, N, N, GL_RGBA32F)
		, noiseSampler(context) {
	std::stringstream ss;
	Bitmap bmp;

	Util::resolveFileLinking(ss, "./src/h0k-shader.glsl", "#include");
	h0kShader = new Shader(context, ss.str());

	for (uint32 i = 0; i < 4; ++i) {
		ss.str("");
		ss << "./res/Noise256_" << i << ".jpg";

		bmp.load(ss.str());
		noise[i] = new Texture(context, bmp, GL_RGBA32F);
	}

	h0kShader->setInt("N", N);
	h0kShader->setInt("L", L);
}

void OceanFFTSeed::setParams(float amplitude, const glm::vec2& direction,
		float intensity, float capillarSuppressFactor) {
	h0kShader->setFloat("amplitude", amplitude);
	h0kShader->setFloat("intensity", intensity);
	h0kShader->setFloat("l", capillarSuppressFactor);

	h0kShader->setVector2f("direction", glm::normalize(direction));

	h0kShader->bindComputeTexture(imageH0k, 0, GL_WRITE_ONLY, GL_RGBA32F);

	h0kShader->setSampler("noise_r0", *noise[0], noiseSampler, 1);
	h0kShader->setSampler("noise_i0", *noise[1], noiseSampler, 2);
	h0kShader->setSampler("noise_r1", *noise[2], noiseSampler, 3);
	h0kShader->setSampler("noise_i1", *noise[3], noiseSampler, 4);

	context->compute(*h0kShader, computeSpace, computeSpace);
}

OceanFFTSeed::~OceanFFTSeed() {
	delete h0kShader;

	for (uint32 i = 0; i < 4; ++i) {
		delete noise[i];
	}
}

OceanFFT::OceanFFT(RenderContext& context, int32 N, int32 L,
			bool choppy, float timeScale)
		: context(&context)
		, N(N)
		, L(L)
		, log2N(std::ilogb(N))
		, choppy(choppy)
		, timeScale(timeScale)
		, altBuffer(false)
		, timeCounter(0.f)
		, fftSeed(context, N, L)
		, butterflyShader(butterflyShader)
		, butterflyTexture(context, log2N, N, GL_RGBA32F)
		, coeffDX(context, N, N, GL_RGBA32F)
		, coeffDY(context, N, N, GL_RGBA32F)
		, coeffDZ(context, N, N, GL_RGBA32F)
		, displacement(context, N, N, GL_RGBA32F)
		, bufferTexture(context, N, N, GL_RGBA32F)
		, foldingMap(context, N, N, GL_RGBA32F)
		, floatingObjectBuffer(context, FLOATING_BUF_SIZE, GL_DYNAMIC_DRAW, 0)
		, floatingObjectSampler(context, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) {
	std::stringstream ss;

	Util::resolveFileLinking(ss, "./src/hkt-shader.glsl", "#include");
	hktShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/butterfly-shader.glsl", "#include");
	butterflyShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/inversion-shader.glsl", "#include");
	inversionShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/folding-shader.glsl", "#include");
	foldingShader = new Shader(context, ss.str());

	ss.str("");
	Util::resolveFileLinking(ss, "./src/ocean-surface-transform.glsl", "#include");
	floatingShader = new Shader(context, ss.str());

	initButterflyTexture(context, N, butterflyTexture);

	hktShader->setInt("N", N);
	hktShader->setInt("L", L);

	inversionShader->setInt("N", N);
	foldingShader->setInt("N", N);
}

void OceanFFT::setOceanParams(float amplitude, const glm::vec2& direction,
		float intensity, float capillarSuppressFactor) {
	fftSeed.setParams(amplitude, direction, intensity, capillarSuppressFactor);
}

void OceanFFT::setFoldingParams(float lambda, float accum, float decay) {
	foldingShader->setFloat("lambda", lambda);
	foldingShader->setFloat("accum", accum);
	foldingShader->setFloat("decay", decay);
}

void OceanFFT::update(float delta) {
	// compute hkt
	hktShader->setFloat("t", timeCounter);

	hktShader->bindComputeTexture(coeffDX, 0, GL_READ_WRITE, GL_RGBA32F);
	hktShader->bindComputeTexture(coeffDY, 1, GL_READ_WRITE, GL_RGBA32F);
	hktShader->bindComputeTexture(coeffDZ, 2, GL_READ_WRITE, GL_RGBA32F);
	
	hktShader->bindComputeTexture(fftSeed.getH0K(), 3, GL_READ_ONLY, GL_RGBA32F);

	context->compute(*hktShader, N / 16, N / 16);
	context->awaitFinish();

	// compute displacement
	computeIFFT(coeffDY, displacement, glm::vec3(0, 1, 0));

	if (choppy) {
		computeIFFT(coeffDX, displacement, glm::vec3(1, 0, 0));
		computeIFFT(coeffDZ, displacement, glm::vec3(0, 0, 1));
	}

	// compute folding map
	foldingShader->bindComputeTexture(displacement, 0, GL_READ_ONLY, GL_RGBA32F);
	foldingShader->bindComputeTexture(foldingMap, 1, GL_READ_WRITE, GL_RGBA32F);
	
	context->compute(*foldingShader, N / 16, N / 16);
	context->awaitFinish();

	// update time counter
	timeCounter += delta * timeScale;

	// update floating objects
	flushFloatingTransforms();
}

OceanFFT::~OceanFFT() {
	delete hktShader;
	delete butterflyShader;
	delete inversionShader;
	delete foldingShader;
	delete floatingShader;
}

inline void OceanFFT::computeIFFT(Texture& coeff, Texture& output,
		const glm::vec3& mask) {
	altBuffer = false;

	butterflyShader->bindComputeTexture(butterflyTexture, 0, GL_READ_ONLY, GL_RGBA32F);
	butterflyShader->bindComputeTexture(coeff, 1, GL_READ_WRITE, GL_RGBA32F);
	butterflyShader->bindComputeTexture(bufferTexture, 2, GL_READ_WRITE, GL_RGBA32F);

	// 1D FFT horizontal
	butterflyShader->setInt("direction", 0);

	for (uint32 i = 0; i < log2N; ++i) {
		butterflyShader->setInt("bufferNum", altBuffer);
		butterflyShader->setInt("stage", i);

		context->compute(*butterflyShader, N / 16, N / 16);
		context->awaitFinish();

		altBuffer = !altBuffer;
	}

	// 1D FFT vertical
	butterflyShader->setInt("direction", 1);

	for (uint32 i = 0; i < log2N; ++i) {
		butterflyShader->setInt("bufferNum", altBuffer);
		butterflyShader->setInt("stage", i);

		context->compute(*butterflyShader, N / 16, N / 16);
		context->awaitFinish();

		altBuffer = !altBuffer;
	}
	
	inversionShader->setInt("bufferNum", altBuffer);
	inversionShader->setVector3f("mask", mask);

	inversionShader->bindComputeTexture(output, 0, GL_READ_WRITE, GL_RGBA32F);

	context->compute(*inversionShader, N / 16, N / 16);
	context->awaitFinish();
}

inline void OceanFFT::flushFloatingTransforms() {
	if (floatingTransforms.size() == 0) {
		return;
	}

	floatingShader->setSampler("displacement", displacement,
			floatingObjectSampler, 0);

	floatingObjectBuffer.update(&floatingTransforms[0],
			floatingTransforms.size() * sizeof(glm::mat4));
	floatingObjectBuffer.update(&floatingSizes[0],
			MAX_FLOATING_OBJECTS * sizeof(glm::mat4),
			floatingSizes.size() * sizeof(glm::vec2));

	context->compute(*floatingShader, floatingTransforms.size());
	context->awaitFinish();

	outputTransforms.resize(floatingTransforms.size());

	void* data = floatingObjectBuffer.map(GL_READ_ONLY);

	std::memcpy(&outputTransforms[0], data,
			floatingTransforms.size() * sizeof(glm::mat4));

	floatingObjectBuffer.unmap();

	floatingTransforms.clear();
	floatingSizes.clear();
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
			GL_STATIC_DRAW, 1, bitReversedIndices);

	// bind uniforms/textures
	btShader.setShaderStorageBuffer("bitReversedIndices", bitReversedBuffer);
	btShader.bindComputeTexture(butterflyTexture, 0, GL_WRITE_ONLY, GL_RGBA32F);

	btShader.setInt("N", N);

	context.compute(btShader, bits, N / 16);
}
