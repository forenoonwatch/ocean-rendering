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

	// TODO: add support for attribs over 4 floats long	
	for (uint32 i = 0; i < 2; ++i) {
		context.setVertexArray(arrays[i]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

		glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);

		for (uint32 attrib = 0, offset = 0; attrib < numAttribs; ++attrib) {
			glEnableVertexAttribArray(attrib);
			glVertexAttribPointer(attrib, attribSizes[attrib], GL_FLOAT, GL_FALSE,
					dataBlockSize, (const void*)(offset * sizeof(float)));

			offset += attribSizes[attrib];
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
