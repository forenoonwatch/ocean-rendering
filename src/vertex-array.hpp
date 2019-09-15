#pragma once

#include "render-context.hpp"
#include "indexed-model.hpp"

class VertexArray {
	public:
		VertexArray(RenderContext& context, const IndexedModel& model, uint32 usage);
		VertexArray(RenderContext& context, const IndexedModel& model,
				VertexArray& vertexArray);

		VertexArray(RenderContext& context, uint32 numBuffers,
				const uint32* elementSizes, uint32 numElements, uint32 usage);

		void updateBuffer(uint32 bufferIndex, const void* data, uintptr dataSize);

		inline const uint32 getBuffer(uint32 bufferIndex);
		inline const uintptr getBufferSize(uint32 bufferIndex) const;

		inline uint32 getID() { return arrayID; }

		inline uint32 getNumBuffers() const { return numBuffers; }
		inline uint32 getNumElements() const { return numElements; }

		inline bool isIndexed() const { return indexed; }

		~VertexArray();
	private:
		NULL_COPY_AND_ASSIGN(VertexArray);

		RenderContext* context;

		uint32 arrayID;

		uint32 numBuffers;
		uint32 numElements;
		uint32 instancedComponentStartIndex;

		uint32 numOwnedBuffers;

		GLuint* buffers;
		uintptr* bufferSizes;

		uint32 usage;

		bool indexed;

		void initVertexBuffers(uint32, const float**, uint32,
				const uint32*, bool);
		void initEmptyArrayBuffers(uint32, uint32, const uint32*);
};

inline const uint32 VertexArray::getBuffer(uint32 bufferIndex) {
	 return buffers[bufferIndex];
}

inline const uintptr VertexArray::getBufferSize(uint32 bufferIndex) const {
	return bufferSizes[bufferIndex];
}
