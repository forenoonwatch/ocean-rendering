#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

#include "render-context.hpp"
#include "indexed-model.hpp"

class VertexArray {
	public:
		VertexArray(RenderContext& context, const IndexedModel& model, uint32_t usage);

		void updateBuffer(uint32_t bufferIndex, const void* data, uintptr_t dataSize);
		void draw(uint32_t primitive, uint32_t numInstances);

		inline void bind() { glBindVertexArray(deviceID); }

		inline uint32_t getID() { return deviceID; }
		inline uint32_t getNumIndices() const { return numIndices; }

		~VertexArray();
	private:
		RenderContext* context;

		uint32_t deviceID;

		uint32_t numBuffers;
		uint32_t numIndices;
		uint32_t instancedComponentStartIndex;

		GLuint* buffers;
		uintptr_t* bufferSizes;

		uint32_t usage;

		void initVertexBuffers(uint32_t, const float**, uint32_t,
				const uint32_t*);
};
