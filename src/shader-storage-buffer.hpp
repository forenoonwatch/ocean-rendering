#pragma once

#include "render-context.hpp"

class ShaderStorageBuffer {
	public:
		ShaderStorageBuffer(RenderContext& context, uintptr dataSize,
				uint32 usage, uint32 blockBinding, const void* data = nullptr);

		void update(const void* data, uintptr dataSize);
		void update(const void* data, uintptr offset, uintptr dataSize);
		inline void update(const void* data) { update(data, size); }

		void* map(uint32 access = GL_WRITE_ONLY);
		void* map(uintptr offset, uintptr size, uint32 access = GL_WRITE_ONLY);

		void unmap();

		inline uint32 getID() { return bufferID; }
		inline uint32 getBlockBinding() { return blockBinding; }

		~ShaderStorageBuffer();
	private:
		NULL_COPY_AND_ASSIGN(ShaderStorageBuffer);

		RenderContext* context;
		uint32 bufferID;
		uint32 blockBinding;
		uintptr size;
};
