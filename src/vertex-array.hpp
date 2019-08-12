#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

#include "render-context.hpp"
#include "indexed-model.hpp"

class VertexArray {
	public:
		VertexArray(RenderContext& context, const IndexedModel& model, uint32 usage);

		void updateBuffer(uint32 bufferIndex, const void* data, uintptr dataSize);
		void draw(uint32 primitive, uint32 numInstances);

		inline void bind() { glBindVertexArray(deviceID); }

		inline uint32 getID() { return deviceID; }
		inline uint32 getNumIndices() const { return numIndices; }

		~VertexArray();
	private:
		RenderContext* context;

		uint32 deviceID;

		uint32 numBuffers;
		uint32 numIndices;
		uint32 instancedComponentStartIndex;

		GLuint* buffers;
		uintptr* bufferSizes;

		uint32 usage;

		void initVertexBuffers(uint32, const float**, uint32,
				const uint32*);
};
