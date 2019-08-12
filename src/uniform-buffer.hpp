#pragma once

#include "render-context.hpp"

class UniformBuffer {
	public:
		UniformBuffer(RenderContext& context, uintptr dataSize,
				uint32 usage, const void* data = nullptr);

		void update(const void* data, uintptr dataSize);
		void update(const void* data, uintptr offset, uintptr dataSize);
		inline void update(const void* data) { update(data, size); }

		inline uint32 getID() { return bufferID; }

		~UniformBuffer();
	private:
		RenderContext* context;
		uint32 bufferID;
		uintptr size;
};
