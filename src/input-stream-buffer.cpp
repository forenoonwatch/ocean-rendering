#include "input-stream-buffer.hpp"

#include <cstring>

InputStreamBuffer::InputStreamBuffer(RenderContext& context,
			uint32 numAttribs, const uint32* attribSizes,
			uint32 numElements)
		: context(&context)
		, readBuffer(0)
		, writeBuffer(1)
		, dataBlockSize(0) {
	glGenVertexArrays(2, arrays);
	glGenBuffers(2, buffers);

	for (uint32 i = 0; i < numAttribs; ++i) {
		dataBlockSize += (uintptr)attribSizes[i] * sizeof(float);
	}

	bufferSize = dataBlockSize * (uintptr)numElements;

	for (uint32 i = 0; i < 2; ++i) {
		context.setVertexArray(arrays[i]);
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
	}
}

void InputStreamBuffer::update(const void* data, uintptr dataSize) {
	context->setVertexArray(arrays[readBuffer]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[readBuffer]);

	void* buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	std::memcpy(buffer, data, dataSize);
	std::memset((void*)((uintptr)buffer + dataSize), 0, bufferSize - dataSize);

	glUnmapBuffer(GL_ARRAY_BUFFER);

	readBuffer = writeBuffer;
	writeBuffer = (writeBuffer + 1) & 1;
}

InputStreamBuffer::~InputStreamBuffer() {
	glDeleteBuffers(2, buffers);
	glDeleteVertexArrays(2, arrays);

	context->setVertexArray(0);
}
