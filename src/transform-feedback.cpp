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

	// TODO: add support for attribs over 4 floats long	
	for (uint32 i = 0; i < 2; ++i) {
		context.setVertexArray(arrays[i]);
		context.setTransformFeedback(feedbacks[i]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

		glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);

		for (uint32 attrib = 0, offset = 0; attrib < numAttribs; ++attrib) {
			glEnableVertexAttribArray(attrib);
			glVertexAttribPointer(attrib, attribSizes[attrib], GL_FLOAT, GL_FALSE,
					dataBlockSize, (const void*)(offset * sizeof(float)));

			offset += attribSizes[attrib];
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
