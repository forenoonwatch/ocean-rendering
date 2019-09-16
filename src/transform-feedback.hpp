#pragma once

#include "render-context.hpp"

class TransformFeedback {
	public:
		TransformFeedback(RenderContext& context, uint32 numAttribsIn,
				const uint32* attribSizesIn, uint32 numElements);

		inline void swapBuffers();

		inline uint32 getFeedback(uint32 i) { return feedbacks[i]; }
		inline uint32 getBuffer(uint32 i) { return buffers[i]; }

		inline uint32 getReadArray() { return arrays[readFeedback]; }
		inline uint32 getWriteArray() { return arrays[writeFeedback]; }
		
		inline uint32 getReadFeedback() { return feedbacks[readFeedback]; }
		inline uint32 getWriteFeedback() { return feedbacks[writeFeedback]; }

		inline uint32 getReadBuffer() { return buffers[writeFeedback]; }
		inline uint32 getWriteBuffer() { return buffers[readFeedback]; }

		inline uint32 getNumAttribs() const { return numAttribs; }
		inline const uint32* getAttribSizes() const { return attribSizes; }
		
		inline uintptr getDataBlockSize() const { return dataBlockSize; }
		inline uintptr getBufferSize() const { return bufferSize; }

		inline uint32 getReadIndex() const { return readFeedback; }
		inline uint32 getWriteIndex() const { return writeFeedback; }

		~TransformFeedback();
	private:
		NULL_COPY_AND_ASSIGN(TransformFeedback);

		RenderContext* context;

		uint32 arrays[2];
		uint32 buffers[2];
		uint32 feedbacks[2];

		uint32 numAttribs;
		uint32* attribSizes;

		uintptr dataBlockSize;
		uintptr bufferSize;

		uint32 readFeedback;
		uint32 writeFeedback;
};

inline void TransformFeedback::swapBuffers() {
	writeFeedback = readFeedback;
	readFeedback = (readFeedback + 1) & 1;
}
