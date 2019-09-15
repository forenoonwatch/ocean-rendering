#pragma once

#include "render-context.hpp"

class TransformFeedback {
	public:
		TransformFeedback(RenderContext& context, uint32 numAttribs,
				const uint32* attribSizes, uint32 numElements);

		inline void flip();

		inline uint32 getFeedback(uint32 i) { return feedbacks[i]; }
		inline uint32 getBuffer(uint32 i) { return buffers[i]; }

		inline uint32 getReadArray() { return arrays[readFeedback]; }
		inline uint32 getWriteArray() { return arrays[writeFeedback]; }
		
		inline uint32 getReadFeedback() { return feedbacks[readFeedback]; }
		inline uint32 getWriteFeedback() { return feedbacks[writeFeedback]; }

		inline uint32 getReadBuffer() { return buffers[writeFeedback]; }
		inline uint32 getWriteBuffer() { return buffers[readFeedback]; }

		inline const uint32* getFeedbacks() { return feedbacks; }

		~TransformFeedback();
	private:
		NULL_COPY_AND_ASSIGN(TransformFeedback);

		RenderContext* context;

		uint32 arrays[2];
		uint32 buffers[2];
		uint32 feedbacks[2];

		uint32 readFeedback;
		uint32 writeFeedback;
};

inline void TransformFeedback::flip() {
	writeFeedback = readFeedback;
	readFeedback = (readFeedback + 1) & 1;
}
