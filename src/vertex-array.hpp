#pragma once

#include "render-context.hpp"
#include "indexed-model.hpp"
#include "transform-feedback.hpp"

class VertexArray {
	public:
		VertexArray(RenderContext& context, const IndexedModel& model, uint32 usage);
		VertexArray(RenderContext& context, const IndexedModel& model,
				VertexArray& vertexArray);

		VertexArray(RenderContext& context, uint32 numBuffers,
				const uint32* elementSizes, uint32 numElements, uint32 usage);

		VertexArray(RenderContext& context, const IndexedModel& model,
				TransformFeedback& tfb, uint32 bufferNum, uint32 usage);
		VertexArray(RenderContext& context, const IndexedModel& model,
				VertexArray& vertexArray, TransformFeedback& tfb, uint32 bufferNum);

		void updateBuffer(uint32 bufferIndex, const void* data, uintptr dataSize);

		inline const uint32 getBuffer(uint32 bufferIndex);
		inline const uintptr getBufferSize(uint32 bufferIndex) const;

		inline uint32 getID() { return arrayID; }

		inline uint32 getNumBuffers() const { return numBuffers; }
		inline uint32 getNumElements() const { return numElements; }

		inline bool isIndexed() const { return indexed; }

		~VertexArray();
	private:
		enum BufferOwnership {
			FULLY_OWNED,
			FULLY_SHARED,
			SHARED_VERTEX_BUFFERS,
			SHARED_INSTANCE_BUFFERS
		};

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

		enum BufferOwnership bufferOwnership;

		void initVertexBuffers(uint32, const float**, uint32,
				const uint32*, bool);
		void initEmptyArrayBuffers(uint32, uint32, const uint32*);
		void initSharedBuffers(uint32, const float**, uint32, const uint32*,
				uint32, const uint32*, uint32, uint32, bool);
};

inline const uint32 VertexArray::getBuffer(uint32 bufferIndex) {
	 return buffers[bufferIndex];
}

inline const uintptr VertexArray::getBufferSize(uint32 bufferIndex) const {
	return bufferSizes[bufferIndex];
}
