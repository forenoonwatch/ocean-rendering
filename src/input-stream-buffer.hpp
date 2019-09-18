#pragma once

#include "render-context.hpp"

class InputStreamBuffer {
	public:
		InputStreamBuffer(RenderContext& context, uint32 numAttribs,
				const uint32* attribSizes, uint32 numElements);

		inline void swapBuffers();

		void update(const void* data, uintptr dataSize);

		inline uint32 getReadArray() { return arrays[readBuffer]; }
		inline uint32 getWriteArray() { return arrays[writeBuffer]; }

		inline uint32 getReadBuffer() { return buffers[readBuffer]; }
		inline uint32 getWriteBuffer() { return buffers[writeBuffer]; }

		inline uint32 getDataBlockSize() const { return dataBlockSize; }

		~InputStreamBuffer();
	private:
		NULL_COPY_AND_ASSIGN(InputStreamBuffer);

		RenderContext* context;

		uint32 arrays[2];
		uint32 buffers[2];

		uint32 readBuffer;
		uint32 writeBuffer;

		uintptr dataBlockSize;
		uintptr bufferSize;
};

inline void InputStreamBuffer::swapBuffers() {
	writeBuffer = readBuffer;
	readBuffer = (readBuffer + 1) & 1;
}
