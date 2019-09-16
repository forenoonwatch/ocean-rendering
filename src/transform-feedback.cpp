#include "transform-feedback.hpp"

TransformFeedback::TransformFeedback(RenderContext& context,
			uint32 numAttribsIn, const uint32* attribSizesIn, uint32 numElements)
		: context(&context)
		, numAttribs(numAttribsIn)
		, attribSizes(new uint32[numAttribsIn])
		, dataBlockSize(0)
		, readFeedback(0)
		, writeFeedback(1) {
	glGenVertexArrays(2, arrays);
	glGenBuffers(2, buffers);
	glGenTransformFeedbacks(2, feedbacks);

	for (uint32 i = 0; i < numAttribs; ++i) {
		attribSizes[i] = attribSizesIn[i];
		dataBlockSize += (uintptr)attribSizes[i] * sizeof(float);
	}

	bufferSize = dataBlockSize * (uintptr)numElements;

	for (uint32 i = 0; i < 2; ++i) {
		context.setVertexArray(arrays[i]);
		context.setTransformFeedback(feedbacks[i]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

		glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);

		uint32 offset = 0;
		uint32 attribute = 0;

		for (uint32 j = 0; j < numAttribs; ++j) {
			const uint32 elementSize = attribSizes[j];
			const uint32 elementSizeDiv = elementSize / 4;
			const uint32 elementSizeRem = elementSize % 4;

			for (uint32 k = 0; k < elementSizeDiv; ++k) {
				glEnableVertexAttribArray(attribute);
				glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE,
						dataBlockSize, (const void*)(offset));

				offset += 4 * sizeof(float);
				++attribute;
			}

			if (elementSizeRem != 0) {
				glEnableVertexAttribArray(attribute);
				glVertexAttribPointer(attribute, elementSizeRem, GL_FLOAT, GL_FALSE,
						dataBlockSize, (const void*)(offset));

				offset += elementSizeRem * sizeof(float);
				++attribute;
			}
		}

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[i]);
	}
}

TransformFeedback::~TransformFeedback() {
	glDeleteTransformFeedbacks(2, feedbacks);
	glDeleteBuffers(2, buffers);
	glDeleteVertexArrays(2, arrays);
	
	context->setVertexArray(0);
	context->setTransformFeedback(0);

	delete[] attribSizes;
}
